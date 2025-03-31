// Author:           Michael Hu
// GG#               1
// cslogin:          mhu

#define GLOBAL_ROW 128
#define GLOBAL_COL 8

int arr[GLOBAL_ROW][GLOBAL_COL];

int main(void) {
    for (int iter = 0; iter < 100; iter++) {
        for (int row = 0; row < GLOBAL_ROW; row += 64) {
            for (int col = 0; col < GLOBAL_COL; col++) {
                arr[row][col] = iter + row + col;
            }
        }
    }
    return 0;
}