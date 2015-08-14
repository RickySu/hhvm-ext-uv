#ifndef UV_LOOP_DATA_H
#define UV_LOOP_DATA_H

namespace HPHP {
    
    class UVLoopData {
        public:
            uv_loop_t *loop = NULL;
            UVLoopData();
            ~UVLoopData();
            void sweep();
    };    
}

#endif
