/*--------------------------------------------------*/
struct Data {
	unsigned char Buffer[1048576];
	unsigned char prog;
};
/*--------------------------------------------------*/
program TEXT_PROG {
	version TEXT_VERS
	{
		Data Binaryzuj(Data) = 1;
	} = 1;
} = 0x23000001;
/*--------------------------------------------------*/
