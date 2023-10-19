int funch(int x, int y) {
    // x = %edi = -4
    // y = %esi = -8
    // if %edi - %esi <= 0 => %esi, else %edi
    return x <= y ? y : x;
}