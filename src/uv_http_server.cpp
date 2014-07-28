#include "ext.h"
#include <r3/r3.h>

namespace HPHP {
    
    static Array HHVM_METHOD(UVHttpServer, _R3Match, const Array &routes, const String &uri, int64_t method) {        
        Array ret;
        node *n = r3_tree_create(routes.size());
        for (ArrayIter iter(routes); iter; ++iter) {
            Variant key(iter.first());
            String pattern = routes.rvalAt(key).toArray().rvalAt(0).toString();
            int64_t idx = key.toInt64Val();
            if(r3_tree_insert_routel(n, -1, pattern.c_str(), pattern.size(), (void *) idx) == NULL) {
                r3_tree_free(n);
                return ret;
            }
        }
        
        if(r3_tree_compile(n, NULL) != 0) {
            r3_tree_free(n);
            return ret;        
        }
        
        match_entry * entry = match_entry_create(uri.c_str());        
        entry->request_method = method;
        route *matched_route = r3_tree_match_route(n, entry);
                
        if(matched_route != NULL) {
            Array params;
            int64_t matched = (int64_t) matched_route->data;
            for(int i=0;i<entry->vars->len;i++){
                params.append(Variant(entry->vars->tokens[i]));
            }
            ret = make_packed_array(matched, params);
        }
        
        match_entry_free(entry);
        r3_tree_free(n);
        return ret;
    }
    
    void uvExtension::_initUVHttpServerClass() {
        HHVM_ME(UVHttpServer, _R3Match);
    }
}