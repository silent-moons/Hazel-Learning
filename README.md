# Hazel-Learning
学习Hazel引擎by Cherno，并尝试修改。  
原作者仓库：https://github.com/TheCherno/Hazel.git  
视频教程：https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT  

### 关于构建
我没有使用Cherno教程中的premake，而是使用了CMake，借助VS2022的CMake解析工具运行项目。

### 功能和笔记
暂时跳过了教程中的两个功能：可视化性能测试（视频55集），SPIR-V着色器中间语言系统（视频100集），其它功能都有详细笔记。整个教程中主要可以学到：  
1. 一个中型项目的架构；
2. 引擎的设计（层、事件、跨平台接口与具体平台实现）；
3. 2D渲染系统，封装了缓冲区、顶点数组、着色器、纹理等渲染资源（目前只支持OpenGL）；
4. 批处理渲染；
5. ECS；
6. 场景序列化与反序列化；
7. 简单的UI与编辑器（使用ImGUI）；
8. 简单的2D物理（使用Box2D）;
9. 基于Mono框架的C#脚本系统。

详见项目中的 Hazel引擎梳理.docx，后续对引擎进行的修改和新增的功能也会加入笔记中。

### 新增功能
1. 3D渲染，使用Renderer为Renderer2D和Renderer3D提供统一接口。新增MeshFilter和MeshRenderer组件配合进行3D渲染，内置了立方体和球体进行分组合批的测试。
2. 实体的树形组织，支持实体父子关系，在层级面板中通过拖动自由设置父子关系。子物体的Transform以父物体为基础，Transform显示的是相对于父物体的平移旋转缩放。
3. 编辑器相机的简单修改，根据当前渲染模式是2D还是3D切换正交和透视模式。