SRCDIR                = .
SUBDIRS               =
DLLS                  = wkWineVSync.dll
LIBS                  =
EXES                  =



### Common settings

CEXTRA                = -m32
CXXEXTRA              = -m32 -std=c++11 -O3
RCEXTRA               =
DEFINES               =
INCLUDE_PATH          =
DLL_PATH              =
DLL_IMPORTS           = ddraw
LIBRARY_PATH          =
LIBRARIES             =


### wkWineVSync.dll sources and settings

wkwinevsync_dll_MODULE= wkWineVSync.dll
wkwinevsync_dll_C_SRCS=
wkwinevsync_dll_CXX_SRCS= main.cpp
wkwinevsync_dll_RC_SRCS=
wkwinevsync_dll_LDFLAGS= -shared \
			-m32
wkwinevsync_dll_ARFLAGS=
wkwinevsync_dll_DLL_PATH=
wkwinevsync_dll_DLLS  =
wkwinevsync_dll_LIBRARY_PATH=
wkwinevsync_dll_LIBRARIES= dxguid GL

wkwinevsync_dll_OBJS  = $(wkwinevsync_dll_C_SRCS:.c=.o) \
			$(wkwinevsync_dll_CXX_SRCS:.cpp=.o) \
			$(wkwinevsync_dll_RC_SRCS:.rc=.res)



### Global source lists

C_SRCS                = $(wkwinevsync_dll_C_SRCS)
CXX_SRCS              = $(wkwinevsync_dll_CXX_SRCS)
RC_SRCS               = $(wkwinevsync_dll_RC_SRCS)


### Tools

CC = winegcc
CXX = wineg++
RC = wrc
AR = ar


### Generic targets

all: $(SUBDIRS) $(DLLS) $(LIBS) $(EXES)

### Build rules

.PHONY: all clean dummy

$(SUBDIRS): dummy
	@cd $@ && $(MAKE)

# Implicit rules

.SUFFIXES: .cpp .cxx .rc .res
DEFINCL = $(INCLUDE_PATH) $(DEFINES) $(OPTIONS)

.c.o:
	$(CC) -c $(CFLAGS) $(CEXTRA) $(DEFINCL) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.rc.res:
	$(RC) $(RCFLAGS) $(RCEXTRA) $(DEFINCL) -fo$@ $<

# Rules for cleaning

CLEAN_FILES     = y.tab.c y.tab.h lex.yy.c core *.orig *.rej \
                  \\\#*\\\# *~ *% .\\\#*

clean:: $(SUBDIRS:%=%/__clean__) $(EXTRASUBDIRS:%=%/__clean__)
	$(RM) $(CLEAN_FILES) $(RC_SRCS:.rc=.res) $(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
	$(RM) $(DLLS) $(LIBS) $(EXES) $(EXES:%=%.so)

$(SUBDIRS:%=%/__clean__): dummy
	cd `dirname $@` && $(MAKE) clean

$(EXTRASUBDIRS:%=%/__clean__): dummy
	-cd `dirname $@` && $(RM) $(CLEAN_FILES)

### Target specific build rules
DEFLIB = $(LIBRARY_PATH) $(LIBRARIES) $(DLL_PATH) $(DLL_IMPORTS:%=-l%)

$(wkwinevsync_dll_MODULE): $(wkwinevsync_dll_OBJS)
	$(CXX) $(wkwinevsync_dll_LDFLAGS) -o $@ $(wkwinevsync_dll_OBJS) $(wkwinevsync_dll_LIBRARY_PATH) $(wkwinevsync_dll_DLL_PATH) $(DEFLIB) $(wkwinevsync_dll_DLLS:%=-l%) $(wkwinevsync_dll_LIBRARIES:%=-l%)
	mv $(wkwinevsync_dll_MODULE).so $(wkwinevsync_dll_MODULE)


