#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"
#include "memory/memory.h"
#include "memory/virtual.h"

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

#include "kernel/utilities/slow.h"
#include "common/string.h"

const char16_t* Song[] =
{
	uR"(Nuapurista kuulu se polokan tahti jalakani pohjii kutkutti, Ievan �iti se tytt��s� vahti vaan kyll�h�n Ieva sen jutkutti, Sill� ei meit� silloin kiellot haittaa, Kun my� tanssimme laiasta laitaan)",

	uR"(Salivili hipput tupput t�ppyt �ppyt tipput hilijalleen)",

	uR"(Ievan suu oli vehn�sell� ko immeiset onnee toevotti
Pe� oli m�rk�n� jokaisella ja viulu se vinku ja voevotti
Ei t�t� poikoo m�rkyys haittaa
Sillon ko laskoo laiasta laitaan)",

	uR"(Salivili hipput tupput t�ppyt �ppyt tipput hilijalleen)",

	uR"(Ievan �iti se kammarissa virsi� veisata huijjuutti, Kun t�m� poika naapurissa �mm�n tytt�� nuijjuutti, Eik� t�t� poikoo �mm�t haittaa, Sillon ko laskoo laiasta laitaan)",

	uR"(Salivili hipput tupput t�ppyt �ppyt tipput hilijalleen)",

	uR"(trololololololololoolololololololololoolololololololololoolololololololololoolololololololololoololololololololololololololololololoolololololololololoolololololololololoolololololololololoolololololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololooloolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololooloolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololooloolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololooloolololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolololololololololoolol)",

	uR"(Hilipati hilipati hilipati hillaa
Hilipati hilipati hilipampaa
Jalituli jallaa talituli jallaa
Tilitali tilitali tilitantaa
Hilipati hillaa hilipati hillaa
Hilipati hilipati jalituli jallaa
Tilitali tallaa, tulituli jallaa
Hilipati hilipati hilipampaa
Rimpatirillaa ripirapirullaa
Rumpatirumpa tiripirampuu
Jamparingaa rimpatiraparan
Tsupantupiran dillandu
Japat stilla dipudupu dullaa
Dumpatidupa lipans dullaa
Dipidapi dullaa rimpati rukan
Ribitit stukan dillandu
Jatsatsa barillas dilla lapadeian dullan deian doo
Joparimba badabadeia stulla
Laba daba daba dujan dillandu
Barillas dilla deiaduu badaba daga daga daga daga dujaduu
Badu dubi dubi dubi dejaduu
Badaba dillas dillan dejaduu)",

	uR"(Siell� oli lysti� soiton j�l�keen sain min� kerran sytkyyttee
Kottiin ko m�ntii ni �mm� se riitelj ja Ieva jo alako nyyhkyytteek
Min� sanon Ievalle mit�p� se haittaa
Laskemma viel�hi laiasta laitaa)",

	uR"(Salivili hipput tupput t�ppyt �ppyt tipput hilijalleen)",

	uR"(Muorille sanon jotta tukkee suusi en ruppee sun terveytt�s takkoomaa
Terveen� pe�set ku korjoot luusi ja m��t siit� murjuus makkoomaa
Ei t�t� poikoo hellyys haittaa
Ko akkoja huhkii laiasta laitaan)",

	uR"(Salivili hipput tupput t�ppyt �ppyt tipput hilijalleen)",

	uR"(Sen min� sanon jotta purra pitt�� ei mua niin voan nielasta
Suat m�nn� ite vaikka l�nnest� itt��n vaan min� en luovu Ievasta
Sill� ei t�t� poikoo kainous haittaa
Sillon ko tanssii laiasta laitaan)"
};

void WriteToMemoryWeOwn(void* Address, uint64_t Size, int Pattern)
{
	memset(Address, Pattern, Size);
}

void WriteToMemoryWeDontOwn(void* Address, uint64_t Size, int Pattern)
{
	memset(Address, Pattern, Size);
}

extern "C" void __attribute__((sysv_abi, __noreturn__)) KernelMain(KernelBootData * BootData)
{
	OnKernelMainHook();

	GBootData = *BootData;

	DefaultConsoleInit(GBootData.Framebuffer, BMFontColor{ 0x0, 0x20, 0x80 }, BMFontColor{ 0x8c, 0xFF, 0x0 });

	ConsolePrint(u"Starting Enkel (Revision ");
	ConsolePrint(KernelBuildId);
	ConsolePrint(u")...\n");

	if (IsDebuggerPresent())
	{
		ConsolePrint(u"DEBUGGER ATTACHED!\n");
	}

	EnterLongMode(&GBootData);
	
	const int Size = 16 * 1024 * 1024;
	void* MyMemory = VirtualAlloc(Size);
	WriteToMemoryWeOwn(MyMemory, Size, 0xFE);

	VirtualProtect(MyMemory, Size, MemoryProtection::ReadOnly);
	WriteToMemoryWeOwn(MyMemory, Size, 0xCC);

	VirtualFree(MyMemory, Size);

	//Should crash here
	WriteToMemoryWeDontOwn(MyMemory, Size, 0xDE);

	//while (true)
	{
		for (size_t Verse = 0; Verse < sizeof(Song) / sizeof(Song[0]); Verse++)
		{
			ConsolePrint(Song[Verse]);

			ConsolePrint(u"\r\n\r\n");
			Slow(Loitering);
		}
	}


	ConsolePrint(u"Exiting\n");

	HaltPermanently();
}
