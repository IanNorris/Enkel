
extern "C"
{
	int abs(int Value)
	{
		return Value < 0 ? -Value : Value;
	}

	long labs(long Value)
	{
		return Value < 0 ? -Value : Value;
	}
}