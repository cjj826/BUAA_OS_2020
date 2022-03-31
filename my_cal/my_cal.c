char _my_getchar();
void _my_putchar(char ch);


void my_cal() {
	char c = _my_getchar();
	int n1 = 0;
	int n2 = 0;
	while (c != '\n') {
		n1 *= 10;
		n1 += c - '0';
		c = _my_getchar();
	}

	c = _my_getchar();

	while (c != '\n') {
        n2 *= 10;
        n2 += c - '0';
        c = _my_getchar();
    }
	
	int res = n1 + n2;
	if (res == 0) {
		_my_putchar('0');	
	} else {
		int base = 1000000000;
		while (res / base == 0) base = base / 10;
		while (base) {
			_my_putchar(res / base + '0');
			res = res % base;
			base = base / 10;
		}
	}
}
