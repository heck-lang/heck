//
//  scanner.h
//  CHeckScript
//
//  Created by Mashpoe on 3/12/19.
//

#ifndef scanner_h
#define scanner_h

#include <stdio.h>
#include <stdbool.h>
#include "code.h"

// returns 0 on failure
bool heck_scan(heck_code* c, FILE* f);

#endif /* scanner_h */
