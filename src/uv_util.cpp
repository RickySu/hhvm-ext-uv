#include "ext.h"

namespace HPHP {

    static int64_t HHVM_STATIC_METHOD(UVUtil, version) {
        return uv_version();
    }
    
    static String HHVM_STATIC_METHOD(UVUtil, errorMessage, int64_t err) {
        return String(uv_strerror(err), CopyString);
    }    
    
    static String HHVM_STATIC_METHOD(UVUtil, versionString) {
        return String(uv_version_string(), CopyString);
    }    
    
    void uvExtension::_initUVUtilClass() {
        HHVM_STATIC_ME(UVUtil, errorMessage);    
        HHVM_STATIC_ME(UVUtil, version);
        HHVM_STATIC_ME(UVUtil, versionString);
    }
}