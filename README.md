[中文README](README_zh.md)

This is a php debug/trace tool. It can trace php execution, then output traced information to file. [ytrace_gui](https://github.com/yangxikun/ytrace_gui) can load traced file, show execution detail and debug the execution. See also [ytrace_chrome_extension](https://github.com/yangxikun/ytrace_chrome_extension)

## install
* git clone this project
* phpize
* ./configure --enable-ytrace
* make test (please report issue if you encounter any failed test)
* make install

## Wiki
__INI config__
+ auto_enable: Type: boolean, Default value: 0, PHP_INI_SYSTEM.
+ enable_trigger: Type: boolean, Default value: 0, PHP_INI_SYSTEM.
  - When this setting is set to 1, you can trigger the generation of trace files by using the YTRACE_TRIGGER GET/POST parameter, or set a cookie with the name YTRACE_TRIGGER, or set an environment variable with the name YTRACE_TRIGGER.
+ enable_trigger_name: Type: string, Default value: YTRACE_TRIGGER, PHP_INI_SYSTEM.
+ enable_trigger_value: Type: string, Defalut value: "", PHP_INI_SYSTEM.
  - This setting can be used to restrict who can make use of the YTRACE_TRIGGER functionality as outlined in ytrace.enable_trigger.
  - When changed from its default value of an empty string, the value of the cookie, environment variable, GET or POST argument needs to match the shared secret set with this setting in order for the trace file to be generated.
+ output_dir: Type: string, Default value: /tmp, PHP_INI_SYSTEM.
  - make sure has write permission.
+ output_format: Type: string, Default value: trace-%t, PHP_INI_SYSTEM.
  - name format of traced file
  - %t: timestamp (in seconds)
  - %u: timestamp (in microseconds)
  - %p: pid
  - %H: $_SERVER['HTTP_HOST']
  - %U: $_SERVER['UNIQUE_ID']
  - %R: $_SERVER['REQUEST_URI']
  - %%: literal %
+ white_list: Type: string, Default value: "", PHP_INI_ALL.
  - comma separated string, case sensitive.
  - When set its value, sunch as "controller,model", only executed source file path that contain "controller" or "model" will be traced.
  - white_list takes precedence over black_list.
+ white_list_name: Type: string, Default value: "YTRACE_WHITE_LIST", PHP_INI_SYSTEM.
+ black_list: Type: string, Default value: "", PHP_INI_ALL.
  - comma separated string, case sensitive.
  - When set its value, sunch as "vendor,lib", executed source file path that contain "vendor" or "lib" will not be traced.
+ black_list_name: Type: string, Default value: "YTRACE_BLACK_LIST", PHP_INI_SYSTEM.
+ var_display_max_children: Type: integer, Default value: 128, PHP_INI_ALL.
  - Controls the amount of array children and object's properties are traced.
  - max value 32
+ var_display_max_children_name Type: string, Default value: "YTRACE_VAR_DISPLAY_MAX_CHILDREN", PHP_INI_SYSTEM.
+ var_display_max_data: Type: integer, Default value: 512, PHP_INI_ALL.
  - Controls the maximum string length that is traced.
  - max value 1024
+ var_display_max_data_name Type: string, Default value: "YTRACE_VAR_DISPLAY_MAX_DATA", PHP_INI_SYSTEM.
+ var_display_max_depth: Type: integer, Default value: 3, PHP_INI_ALL.
  - Controls how many nested levels of array elements and object properties are traced.
  - max value 16
+ var_display_max_depth_name Type: string, Default value: "YTRACE_VAR_DISPLAY_MAX_DEPTH", PHP_INI_SYSTEM.

> Generally, you don’t need to change the default config of *_name. The value of *_name is used for the name of cookie, environment variable, GET or POST argument

__PHP function__
+ ytrace_enable ([$traced_file_name])
  - enable trace when trace is disable, write trace to $traced_file_name.
+ ytrace_disable ()
  - disable trace when trace is enable, return traced file.
