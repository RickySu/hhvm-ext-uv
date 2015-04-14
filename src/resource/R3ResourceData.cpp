/*
 * File:   R3ResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "R3ResourceData.h"

namespace HPHP {

    RESOURCEDATA_ALLOCATION(R3ResourceData)
    
    R3ResourceData::R3ResourceData(int count) {
        n = r3_tree_create(count);
    }

    R3ResourceData::~R3ResourceData() {
        r3_tree_free(n);
    }
    
    node *R3ResourceData::getR3ResourceData(){
        return n;
    }

}
