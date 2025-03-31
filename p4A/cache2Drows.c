// Author:           Michael Hu
// GG#               1
// cslogin:          mhu

#define GLOBAL_ROW 3000
#define GLOBAL_COL 500

int arr[GLOBAL_ROW][GLOBAL_COL];

int main(void) {
    for (int row = 0; row < GLOBAL_ROW; row++) {
        for (int col = 0; col < GLOBAL_COL; col++) {
            arr[row][col] = row + col;
        }
    }
    return 0;
}