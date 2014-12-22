# name of your application
APPLICATION = ha_host

# If no BOARD is found in the environment, use this default:
BOARD ?= mboard-1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../RIOT

# Uncomment these lines if you want to use platform support from external
# repositories:
#RIOTCPU ?= $(CURDIR)/../../../thirdparty_cpu
#RIOTBOARD ?= $(CURDIR)/../../../thirdparty_boards

# Uncomment this to enable scheduler statistics for ps:
#CFLAGS += -DSCHEDSTATISTICS

# If you want to use native with valgrind, you should recompile native
# with the target all-valgrind instead of all:
# make -B clean all-valgrind

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Blacklist boards
BOARD_BLACKLIST := arduino-due avsextrem chronos mbed_lpc1768 msb-430h msba2 redbee-econotag \
                   telosb wsn430-v1_3b wsn430-v1_4 msb-430 pttu udoo qemu-i386 z1 stm32f0discovery \
                   stm32f3discovery stm32f4discovery pca10000 pca10005

# This example only works with native for now.
# msb430-based boards: msp430-g++ is not provided in mspgcc.
# (People who want use c++ can build c++ compiler from source, or get binaries from Energia http://energia.nu/)
# msba2: some changes should be applied to successfully compile c++. (_kill_r, _kill, __dso_handle)
# stm32f0discovery: g++ does not support some used flags (e.g. -mthumb...)
# stm32f3discovery: g++ does not support some used flags (e.g. -mthumb...)
# stm32f4discovery: g++ does not support some used flags (e.g. -mthumb...)
# pca10000:         g++ does not support some used flags (e.g. -mthumb...)
# pca10005:         g++ does not support some used flags (e.g. -mthumb...)
# iot-lab_M3: g++ does not support some used flags (e.g. -mthumb...)
# others: untested.

#----------------------- HA project configuration -----------------------------#

# HA network device type
CFLAGS += -DHA_HOST

# Location for source files and include headers (don't add / in the end)
SRCLOC += ../../libs/MBoard1-libs
SRCLOC += ../../libs/STM32F10x_StdPeriph_Driver/src
SRCLOC += ../../libs/misc
SRCLOC += ../../libs/RIOT-libs/src
SRCLOC += ../../libs/HA-libs
SRCLOC += ../../libs/HA-libs/ha_shell
SRCLOC += ../../libs/HA-libs/ha_sixlowpan
SRCLOC += ../../libs/HA-libs/device_class/src
SRCLOC += ../../libs/HA-libs/device_instance/src
SRCLOC += ../../libs/HA-libs/misc
SRCLOC += ../../libs/FATFileSystem/src
SRCLOC += ha_host
SRCLOC += sixlowpan

INCLOC += ../../libs/MBoard1-libs
INCLOC += ../../libs/STM32F10x_StdPeriph_Driver/inc
INCLOC += ../../libs/misc
INCLOC += ../../libs/RIOT-libs/inc
INCLOC += ../../libs/HA-libs
INCLOC += ../../libs/HA-libs/ha_shell
INCLOC += ../../libs/HA-libs/ha_sixlowpan
INCLOC += ../../libs/HA-libs/device_class/inc
INCLOC += ../../libs/HA-libs/device_instance/inc
INCLOC += ../../libs/HA-libs/common_def
INCLOC += ../../libs/HA-libs/misc
INCLOC += ../../libs/FATFileSystem/src
INCLOC += ha_host
INCLOC += sixlowpan

export CPPMIX =1

# RIOT's usemodules (auto_init is one of default modules)
USEMODULE += uart0
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += vtimer
USEMODULE += udp
USEMODULE += rpl
USEMODULE += defaulttransceiver

# If you want to add some extra flags when compile c++ files, add these flags
# to CXXEXFLAGS variable
CXXEXFLAGS += -fno-exceptions -fno-rtti -std=gnu++11
CFLAGS += -DUSE_STDPERIPH_DRIVER -ffunction-sections -fdata-sections
LINKFLAGS += -Wl,--gc-sections -u _printf_float -u _scanf_float

#----------------------- HA project config processing -------------------------#
# Collect ha modules
USEMODULE += $(notdir $(SRCLOC))
DIRS += $(SRCLOC)

# Add include header to RIOT's INCLUDES
export INCLUDES += $(addprefix -I${CURDIR}/, $(INCLOC))

include $(RIOTBASE)/Makefile.include
