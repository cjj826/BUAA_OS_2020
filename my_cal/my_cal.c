char _my_getchar();
void _my_putchar(char ch);


void my_cal() {
	char c = _my_getchar();
	int res[1000];
	int tot = 0;

	int num = 0;
	while (c != '\n') {
		num *= 10;
		num = num +  c - '0';
		c = _my_getchar();
	}

	if (num == 0) {
		_my_putchar('0');
	} else {
		while (num/2) {
			res[tot++] = num % 2;
			num = num / 2;
		}
		res[tot] = num;
		int i;
		for (i = tot; i >= 0; i--) {
			if (res[i] != 0) 
				break;
		}
		for ( ; i >= 0 ; i--) {
			_my_putchar(res[i] + '0');
		} 
	} 
}
