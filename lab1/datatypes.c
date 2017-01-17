#include <stdio.h>

#define FIXED_POINT_DIVISOR 65536.0

void hex_dump(unsigned char* f,int size);

void main()
{
	unsigned int biggest_uint = 1,i,next_uint = 1;
	signed int biggest_short_int = 1, next_short_int = 1;  
	signed int biggest_int = 1,next_int = 1,smallest_int = -1;
	unsigned char biggest_uchar = 1, next_uchar = 1;
	signed char biggest_char = 1, next_char = 1,smallest_char = -1;
	signed long biggest_long = 1,smallest_long = -1,next_long = 1;
	unsigned long biggest_ulong = 1, next_ulong = 1;
	float sum_pennies = 0.0, sum_dimes = 0.0,penny = 0.01,dime = 0.1,sum1a,sum2a,sum1b,sum2b;
	while((next_char <<= 1) > 0)
	{
		biggest_char = (biggest_char << 1) + 1;
	}
	printf("Largest signed char = %d\n",biggest_char);
	
	while((next_uchar <<= 1) > 0)
	{
		biggest_uchar = (biggest_uchar << 1) + 1;
	}
	printf("Largest unsigned char = %d\n",biggest_uchar);

	
	while((next_int <<= 1) > 0)
	{
		biggest_int = (biggest_int << 1) + 1;		
	}
	printf("Largest signed int = %d\n",biggest_int);
	
	while((next_uint <<= 1) > 0)
	{
		biggest_uint = (biggest_uint << 1) + 1;		
	}
	printf("Largest unsigned int = %u\n",biggest_uint);

	for(next_int=-2;next_int<0;next_int<<=1)
	{
		smallest_int<<=1;
	}
	printf("Smallest signed int = %d\n",smallest_int);

	for(next_char=-2;next_char<0;next_char<<=1)
	{
		smallest_char<<=1;
	}
	printf("Smallest signed char = %d\n",smallest_char);
	
	while((next_long <<= 1) > 0)
	{
		biggest_long = (biggest_long << 1) + 1;
	}
	printf("Largest signed long = %ld\n",biggest_long);
	
	while((next_ulong <<= 1) > 0)
	{
		biggest_ulong = (biggest_ulong << 1) + 1;
	}
	printf("Largest unsigned long = %lu\n",biggest_ulong);

	for(next_long=-2;next_long<0;next_long<<=1)
	{
		smallest_long<<=1;
	}
	printf("Smallest signed long = %ld\n",smallest_long);

	for(i=0;i<15;i++)
	{
		biggest_short_int = (biggest_short_int << 1) + 1;
	}
	printf("Biggest Q16.16 number: %f\n Smallest Q16.16 number: %f\n",((biggest_int)/FIXED_POINT_DIVISOR),((smallest_int)/FIXED_POINT_DIVISOR));
	
	biggest_uchar++;
	biggest_uint++;
	biggest_ulong++;
	printf("Biggest unsigned char plus 1 = %d\nBiggest unsigned int plus 1 = %d\nBiggest unsigned long plus 1 = %lu\n",
	       biggest_uchar,biggest_uint,biggest_ulong);
	

	
	for(i = 0;i<100;i++) 
	{
		sum_pennies += penny;
		sum_dimes += dime;
	}
	printf("Sum of 100 pennies = %f\nBut 100*penny = %f\nSum of 100 dimes = %f\nBut 100*dime = %f\n",
	       sum_pennies,100*penny,sum_dimes,100*dime);

	sum1a = (100+0.0000001)-100;
	sum2a = 0.0000001+(100-100);
	sum1b = (1000000+0.0000003) - 1000000;
	sum2b = 0.0000003 + (1000000-1000000);
	printf("(100+0.0000001)-100 = %f ",sum1a);
	hex_dump((char*)&sum1a,sizeof(float));
	printf("\nBut 0.0000001+(100-100) = %f) ",sum2a);
	hex_dump((char*)&sum2a,sizeof(float));
	printf("\n(1000000+0.0000003) - 1000000 = %f ",sum1b);
	hex_dump((char*)&sum1b,sizeof(float));
	printf("\nBut 0.0000003 + (1000000-1000000) = %f ",sum2b);
	hex_dump((char*)&sum2b,sizeof(float));
	printf("\nThe hex expansions show differences.\n");

}

void hex_dump(unsigned char* f,int size)
{
	int k;
	printf("hex:");
	for (k = 0; k<size;k++)
	{
		printf("%02x ",f[k]);
	}
}