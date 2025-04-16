# Hazel-Learning
学习Hazel引擎by Cherno，并尝试修改。  
原作者仓库：https://github.com/TheCherno/Hazel.git  
视频教程：https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT  

### 关于构建
我没有使用Cherno教程中的premake，而是使用了CMake，借助VS2022的CMake解析工具运行项目。

### 功能和笔记
暂时跳过了教程中的两个功能：可视化性能测试（视频55集开始），SPIR-V着色器中间语言系统（视频100集）。其它功能都有详细笔记，详见项目中的 Hazel引擎梳理.docx，后续对引擎进行的修改和新增的功能也会加入笔记中。

### 新增功能
1. 3D渲染，使用Renderer为Renderer2D和Renderer3D提供统一接口。新增MeshFilter和MeshRenderer组件配合进行3D渲染，内置了立方体和球体进行分组合批的测试。