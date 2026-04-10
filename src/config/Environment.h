#pragma once

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define FW_VERSION     APP_VERSION
#define FW_BUILD_DATE  __DATE__
#define FW_BUILD_TIME  __TIME__
#define FW_GIT_HASH    GIT_HASH