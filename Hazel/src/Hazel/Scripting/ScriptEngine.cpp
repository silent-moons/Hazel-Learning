#include "hzpch.h"
#include "ScriptEngine.h"

#include <fstream>

#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"

namespace Hazel 
{
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },
		{ "Hazel.Vector2", ScriptFieldType::Vector2 },
		{ "Hazel.Vector3", ScriptFieldType::Vector3 },
		{ "Hazel.Vector4", ScriptFieldType::Vector4 },
		{ "Hazel.Entity", ScriptFieldType::Entity },
	};

	namespace Utils {

		// TODO: move to FileSystem class
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint64_t size = end - stream.tellg();

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = (uint32_t)size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			// 读取程序集到内存
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// 加载程序集的视图，里面包含程序集的元数据，是MonoAssembly的构建基础
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);
			if (status != MONO_IMAGE_OK) // 视图加载失败，记录错误
			{
				const char* errorMessage = mono_image_strerror(status);
				HZ_CORE_ERROR("Failed to load assembly: {0}, Error: {1}", 
					assemblyPath.string(), errorMessage);
				return nullptr;
			}

			// 根据前面打开的MonoImage，创建一个实际的运行时MonoAssembly
			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);
			delete[] fileData;

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				HZ_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);
			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				HZ_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}
			return it->second;
		}

		const char* ScriptFieldTypeToString(ScriptFieldType type)
		{
			switch (type)
			{
			case ScriptFieldType::Float:   return "Float";
			case ScriptFieldType::Double:  return "Double";
			case ScriptFieldType::Bool:    return "Bool";
			case ScriptFieldType::Char:    return "Char";
			case ScriptFieldType::Byte:    return "Byte";
			case ScriptFieldType::Short:   return "Short";
			case ScriptFieldType::Int:     return "Int";
			case ScriptFieldType::Long:    return "Long";
			case ScriptFieldType::UByte:   return "UByte";
			case ScriptFieldType::UShort:  return "UShort";
			case ScriptFieldType::UInt:    return "UInt";
			case ScriptFieldType::ULong:   return "ULong";
			case ScriptFieldType::Vector2: return "Vector2";
			case ScriptFieldType::Vector3: return "Vector3";
			case ScriptFieldType::Vector4: return "Vector4";
			case ScriptFieldType::Entity:  return "Entity";
			}
			return "<Invalid>";
		}

	}

	struct ScriptEngineData
	{
		// 运行C#脚本的上下文环境
		MonoDomain* RootDomain = nullptr;
		// 当前运行的域，相当于一个沙盒，可以安全地加载和运行一组程序集，与其他域互不干扰
		MonoDomain* AppDomain = nullptr; 

		// C#项目核心程序集，提供向量，组件等操作，相当于引擎功能的C#接口
		MonoAssembly* CoreAssembly = nullptr;
		// C#项目核心程序集的视图，包含了程序集的元数据
		MonoImage* CoreAssemblyImage = nullptr;
		// C#项目用户程序集，是用户自定义的逻辑
		MonoAssembly* AppAssembly = nullptr;
		// C#项目用户程序集的视图
		MonoImage* AppAssemblyImage = nullptr;

		// C#项目核心程序集中的Entity类，提供了实体的基本操作
		ScriptClass EntityClass;
		// C#项目用户程序集中的所有Entity类的子类，{类名，ScriptClass对象}
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		// C#项目用户程序集中的所有Entity类的子类实例化的对象，{UUID，ScriptInstance对象}
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		Scene* SceneContext = nullptr; // Runtime
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		HZ_CORE_WARN("Script System!");
		s_Data = new ScriptEngineData();

		InitMono(); // 初始化mono
		LoadAssembly("Resources/Scripts/Hazel-ScriptCore.dll"); // 加载C#核心程序集
		LoadAppAssembly("Resources/Scripts/SandboxProject.dll"); // 加载C#用户程序集
		LoadAssemblyClasses(); // 加载C#中所有Entity类的子类，存入map

		ScriptGlue::RegisterComponents(); // 注册C#类中的所有组件
		ScriptGlue::RegisterFunctions(); // 将C++的函数注册到C#
		s_Data->EntityClass = ScriptClass("Hazel", "Entity", true);
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_Data;
	}

	void ScriptEngine::InitMono()
	{
		// 加载mono运行时，内含C#的内置程序集和库，类似于C++的std
		mono_set_assemblies_path("mono/lib"); 
		// 启动Mono的jit，创建一个根域，HazelJITRuntime是jit实例的名称，可以任意命名
		MonoDomain* rootDomain = mono_jit_init("HazelJITRuntime");
		HZ_CORE_ASSERT(rootDomain, "Mono Jit Init Failed!");
		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		// NOTE(Yan): mono is a little confusing to shutdown, so maybe come back to this

		// mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		// mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// 创建App域
		s_Data->AppDomain = mono_domain_create_appdomain("HazelScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// 加载C#核心项目的dll程序集
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		// 程序集的视图，程序集相当于是代码文件，视图就是代码文件的元数据
		// 后续获取程序集的类、方法等信息都需要从视图中获取
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// 加载C#用户项目的dll程序集
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();

		// 组件的脚本名称是否正确，ClassName来自编辑器脚本组件的输入框
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			// 实例化类对象，并存储OnCreate、OnUpdate函数，调用父类Entity的构造函数，传入实体的UUID
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
			// 存储mono脚本对象，即C#中的Player，Camera等
			s_Data->EntityInstances[entity.GetUUID()] = instance;
			instance->InvokeOnCreate(); // 调用C#的OnCreate函数
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();
		HZ_CORE_ASSERT(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end(), "");

		// 根据实体UUID获取mono对象的指针
		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate((float)ts); // 调用C#的OnUpdate函数
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end())
			return nullptr;
		return it->second;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;

		s_Data->EntityInstances.clear();
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		// 根据命名空间、类名、MonoImage加载C#的Mono类，这里加载Entity父类
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Hazel", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			// 加载APP的Dll中所有C#类
			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			// entity父类不保存
			if (monoClass == entityClass)
				continue;

			// 判断当前类是否为Entity的子类，是就存入脚本map中
			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
			if (!isEntity)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;
			int fieldCount = mono_class_num_fields(monoClass); // 读取脚本类的属性
			HZ_CORE_INFO("Loading user scripts: {} has {} fields:", className, fieldCount);
			void* iterator = nullptr;

			// 遍历所有属性
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				// 获取属性的名称，即C#脚本里的变量名
				const char* fieldName = mono_field_get_name(field); 
				uint32_t flags = mono_field_get_flags(field); // 获取属性的权限
				if (flags & FIELD_ATTRIBUTE_PUBLIC) // 公有属性
				{
					MonoType* type = mono_field_get_type(field); // 获取属性的类类型

					// 将MonoType类型（如System.Single）转化为更好识别的类型（如Float）
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					HZ_CORE_INFO("public field: {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));
					// 设置属性名对应的结构体，写入该属性的信息
					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate(); // 实例化mono对象

		// 获取父类（Entity）类的构造函数，OnCreate，OnUpdate
		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// 调用Entity类的构造函数，传入实体ID
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields(); // 存储属性信息的map
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second; // 属性的信息
		mono_field_get_value(m_Instance, field.ClassField, buffer); // 将属性的值写入buffer
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields(); // 存储属性信息的map
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second; // 属性的信息
		mono_field_set_value(m_Instance, field.ClassField, (void*)value); // 使用value改变属性的值
		return true;
	}
}