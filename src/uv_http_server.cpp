#include "ext.h"
#include <r3/r3.h>

namespace HPHP {
    
    static void HHVM_METHOD(UVHttpServer, __construct) {
        Resource resource(NEWOBJ(R3ResourceData(10)));
        SET_RESOURCE(this_, resource, s_uvhttpserver);
//        r3_tree_compile(r3_node->n, NULL);        
//        r3_tree_dump(r3_node->n, 0);
    }
    
    static void HHVM_METHOD(UVHttpServer, _R3Compile){        
        R3ResourceData *resource = FETCH_RESOURCE(this_, R3ResourceData, s_uvhttpserver);
        node *n = resource->getR3ResourceData();
        r3_tree_compile(n, NULL);
    }
    
    void uvExtension::_initUVHttpServerClass() {
        HHVM_ME(UVHttpServer, __construct);
        HHVM_ME(UVHttpServer, _R3Compile);        
    }
}