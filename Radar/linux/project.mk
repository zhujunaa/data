#请修改以下内容以确定相关内容

#项目类型(可执行程序exe,静态库lib,动态库dll,so)
export PROJECT_TYPE=exe
export PROJECT_AUTHOR=E15
export PROJECT_VERSION=1.0.0.0

E15_PUBLIC_FILE=

#源文件所在路径 （目录列表）
PROJECT_SOURCE= ../gtk ../draw_dia/ ../src ../client ../../public/data_mgr/ ../../public/store/

#头文件路径(-I目录)
PROJECT_INCLUDE= -I/data/runtime/cxx_inc \
		 -I../gtk \
		 -I../draw_dia \
		 -I../src/ \
		 -I../client/ \
		 -I../../market/include \
		 -I../../public/stock_inc \
		 -I../../public/data_mgr/ \
		 -I../../public/store \
		 -I/usr/include/gtk-3.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/gtk-3.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/pango-1.0 -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libdrm -I/usr/include/libpng16 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/libpng16 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include 



#版本文件名称
PROJECT_VERSION_H= 

#其他的链接路径，(-L目录)
PROJECT_LINK_DIR= -L/data/runtime/linux/so \

#工程的宏定义
PROJECT_DEF= -rdynamic

# debug 工程的宏定义
export PROJECT_DEF_DEBUG= -D_DEBUG -DDEBUG  
# release 工程的宏定义
export PROJECT_DEF_RELEASE= -DNDEBUG

#工程需要链接的其他动态库
export PROJECT_IMPORT_DLL= -rdynamic -ldl -lc \
		-lgtk-3 -lgdk-3 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0

# debug 工程需要链接的其他静态库文件列表
export PROJECT_IMPORT_LIB_DEBUG= -lE15_cxx_objectD -lE15_stock_analyseD
#-lE15_stock_analyseD
export PROJECT_IMPORT_LIB_RELEASE= -lE15_cxx_object -lE15_stock_analyse
#-lE15_stock_analyse

#项目编译中间obj文件路径
export PROEJCT_OUT_PATH = /tmp/
#项目编译输出路径
export PROJECT_PATH = /data/runtime/linux/stock/radar
#项目输出文件主名称(不带后缀）
export PROJECT_BASENAME = radar
export PROJECT_OUT_PATH=/tmp/e15_build/linux/$(PROJECT_BASENAME)
