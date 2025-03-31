
// Author:           Michael Hu
// GG#               1
// cslogin:          mhu

#define GLOBAL_1D 100000

int arr[GLOBAL_1D];

int main(void) {
    int i;
    for (i = 0; i < GLOBAL_1D; i++) {
        arr[i] = i;
    }
    return 0;
}