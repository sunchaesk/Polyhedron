int main() {
    int x[5] = {0,0,0,0,0};
    // x[0] = 1;
    // x[1] = 2;
    for (int i = 0; i < 10; i++){
        for (int j = 0; j < 10; j++){
            x[i*j] = i*j;
        }
    }

    // int N = 10;
    // for (int j = 0; j < 100; j++){
    //     for (int i = 0-10*j; i < 10; i-=1){
    //         for (int z = N * j; z < 100; z++){

    //         }
    //     }
    // }
}
