这是一个PHP调试/跟踪根据。它能够记录下PHP代码的执行过程，并输出到文件中。[ytrace_gui](https://github.com/yangxikun/ytrace_gui)能够加载跟踪文件，展示执行详细信息，以及调试执行过程。相关的还有[ytrace_chrome_extension](https://github.com/yangxikun/ytrace_chrome_extension)。

## 安装
需要 php >= 5.5
* git clone 当前项目
* phpize
* ./configure --enable-ytrace
* make test (如果出现失败的测试用例，请提交issue)
* make install

## Wiki
__INI 配置__
+ auto_enable: 类型：boolean，默认值：0, PHP_INI_SYSTEM.
+ enable_trigger: 类型：boolean，默认值：0, PHP_INI_SYSTEM.
  - 当设置为1时，你可以通过名为YTRACE_TRIGGER的GET/POST参数，或cookie，或环境变量来触发跟踪
+ enable_trigger_name: 类型：string，默认值：YTRACE_TRIGGER, PHP_INI_SYSTEM.
+ enable_trigger_value: 类型：string，默认值："", PHP_INI_SYSTEM.
  - 用于限制触发跟踪的值
  - 当为空字符串时，只要检测到有YTRACE_TRIGGER名称的GET/POST参数，或cookie，或环境变量时，就会触发跟踪
  - 当为非空字符串时，名称为YTRACE_TRIGGER的GET/POST参数，或cookie，或环境变量对应的值必须与之匹配
+ output_dir: 类型：string，默认值： /tmp, PHP_INI_SYSTEM.
  - 确保有写权限
+ output_format: 类型：string，默认值： trace-%t, PHP_INI_SYSTEM.
  - 跟踪文件命名
  - %t: 秒级时间戳
  - %u: 毫秒级时间戳
  - %p: pid
  - %H: $_SERVER['HTTP_HOST']
  - %U: $_SERVER['UNIQUE_ID']
  - %R: $_SERVER['REQUEST_URI']
  - %%: %字面量
+ white_list: 类型：string，默认值： "", PHP_INI_ALL.
  - 由逗号分隔的多个字符串组成，大小写敏感
  - 当设置它的值时，例如“controller,model”，那么只有源码文件路径包含“controller”或“model”，才会被跟踪
  - white_list优先级高于black_list
+ white_list_name: 类型：string，默认值： "YTRACE_WHITE_LIST", PHP_INI_SYSTEM.
+ black_list: 类型：string，默认值： "", PHP_INI_ALL.
  - 由逗号分隔的多个字符串组成，大小写敏感
  - 当设置它的值时，例如“vendor,lib”，如果源码文件路径包含“vendor”或“lib”，将不会被跟踪
+ black_list_name: 类型：string，默认值： "YTRACE_BLACK_LIST", PHP_INI_SYSTEM.
+ var_display_max_children: 类型：integer，默认值： 128, PHP_INI_ALL.
  - Controls the amount of array children and object's properties are traced.
  - 控制跟踪到的变量值最大的数组元素个数和对象的属性个数
  - 最大值是 32
+ var_display_max_children_name 类型：string，默认值： "YTRACE_VAR_DISPLAY_MAX_CHILDREN", PHP_INI_SYSTEM.
+ var_display_max_data: 类型：integer，默认值： 512, PHP_INI_ALL.
  - Controls the maximum string length that is traced.
  - 控制跟踪到的字符串变量值的最大长度
  - 最大值 1024
+ var_display_max_data_name 类型：string，默认值： "YTRACE_VAR_DISPLAY_MAX_DATA", PHP_INI_SYSTEM.
+ var_display_max_depth: 类型：integer，默认值： 3, PHP_INI_ALL.
  - 控制跟踪到的变量值的最大嵌套层级
  - 最大 16
+ var_display_max_depth_name 类型：string，默认值： "YTRACE_VAR_DISPLAY_MAX_DEPTH", PHP_INI_SYSTEM.

> 通常，你不需要修改*_name配置的默认值。*_name的配置是用于设置cookie、环境变量、GET/POST参数的名称。

__PHP 函数__
+ ytrace_enable ([$traced_file_name])
  - 开启跟踪，并写入到$traced_file_name
+ ytrace_disable ()
  - 停止跟踪，返回跟踪文件名iki
