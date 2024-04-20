#pragma once

template <typename T, size_t Size, bool DropOnFull = false>
class RingBuffer
{
public:
	RingBuffer()
	: Head(0)
	, Tail(0)
	, IsFull(false)
	{}

	bool Push(const T& item)
	{
		if (IsFull)
		{
			if constexpr (DropOnFull)
			{
				Pop();
			}
			else
			{
				return false;
			}
		}

		Buffer[Head] = item;
		Head = (Head + 1) % Size;

		if (Head == Tail)
		{
			IsFull = true;
		}

		return true;
	}

	bool Pop(T& out)
	{
		if (IsEmpty())
		{
			return false;
		}

		T item = Buffer[Tail];
		Tail = (Tail + 1) % Size;
		IsFull = false;

		out = item;
		return true;
	}

	void Clear()
	{
		Head = 0;
		Tail = 0;
		IsFull = false;
	}

	size_t PopElements(T* outputBuffer, size_t count)
	{
		size_t elementsCount = 0;

		if (!IsEmpty())
		{
			if (IsFull)
			{
				elementsCount = Size;
			}
			else if (Head > Tail)
			{
				elementsCount = Head - Tail;
			}
			else
			{
				elementsCount = Size - Tail + Head;
			}

			if (count < elementsCount)
			{
				elementsCount = count;
			}

			if (Head > Tail)
			{
				for (size_t i = 0; i < elementsCount; ++i)
				{
					outputBuffer[i] = Buffer[Tail + i];
				}
			}
			else
			{
				size_t firstPart = Size - Tail;
				if (firstPart > elementsCount)
				{
					firstPart = elementsCount;
				}

				for (size_t i = 0; i < firstPart; ++i)
				{
					outputBuffer[i] = Buffer[Tail + i];
				}

				for (size_t i = 0; i < elementsCount - firstPart; ++i)
				{
					outputBuffer[firstPart + i] = Buffer[i];
				}
			}

			Tail = (Tail + elementsCount) % Size;
			IsFull = false;
		}

		return elementsCount;
	}

	bool IsEmpty() const
	{
		return !IsFull && (Head == Tail);
	}

private:
	size_t Head;
	size_t Tail;
	bool IsFull;

	T Buffer[Size];

	void Pop()
	{
		if (!IsEmpty())
		{
			Tail = (Tail + 1) % Size;
			IsFull = false;
		}
	}
};
