#ifndef UV_HTTP_SERVER_H
#define UV_HTTP_SERVER_H
#include "ext.h"
#include <r3/r3.h>

namespace HPHP {
    class UVHttpServerData {
        private:
            node *n = NULL;
        public:
            ALWAYS_INLINE node *create(int count){
                n = r3_tree_create(count);
                return n;
            }
            ~UVHttpServerData();
            ALWAYS_INLINE node *getNode(){
                return n;
            }
    };
}

#endif