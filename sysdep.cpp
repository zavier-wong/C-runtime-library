//-------------------------------------------
// This file is used in Linux
// Prevent undefined symbols when linking
//
//-------------------------------------------
extern "C" {
    void* __dso_handle = 0;
}
