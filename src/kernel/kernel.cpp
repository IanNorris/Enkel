#include "utilities/termination.h"
#include "kernel/console/console.h"
#include "kernel/init/bootload.h"
#include "kernel/init/init.h"
#include "kernel/init/interrupts.h"

KernelBootData GBootData;

extern const char16_t* KernelBuildId;

#include "kernel/utilities/slow.h"
#include "common/string.h"

const char16_t* Song[] =
{
	uR"(Nuapurista kuulu se polokan tahti jalakani pohjii kutkutti, Ievan äiti se tyttöösä vahti vaan kyllähän Ieva sen jutkutti, Sillä ei meitä silloin kiellot haittaa, Kun myö tanssimme laiasta laitaan)",

	uR"(Salivili hipput tupput täppyt äppyt tipput hilijalleen)",

	uR"(Ievan suu oli vehnäsellä ko immeiset onnee toevotti
Peä oli märkänä jokaisella ja viulu se vinku ja voevotti
Ei tätä poikoo märkyys haittaa
Sillon ko laskoo laiasta laitaan)",

	uR"(Salivili hipput tupput täppyt äppyt tipput hilijalleen)",

	uR"(Ievan äiti se kammarissa virsiä veisata huijjuutti, Kun tämä poika naapurissa ämmän tyttöä nuijjuutti, Eikä tätä poikoo ämmät haittaa, Sillon ko laskoo laiasta laitaan)",

	uR"(Salivili hipput tupput täppyt äppyt tipput hilijalleen)",

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

	uR"(Siellä oli lystiä soiton jäläkeen sain minä kerran sytkyyttee
Kottiin ko mäntii ni ämmä se riitelj ja Ieva jo alako nyyhkyytteek
Minä sanon Ievalle mitäpä se haittaa
Laskemma vielähi laiasta laitaa)",

	uR"(Salivili hipput tupput täppyt äppyt tipput hilijalleen)",

	uR"(Muorille sanon jotta tukkee suusi en ruppee sun terveyttäs takkoomaa
Terveenä peäset ku korjoot luusi ja määt siitä murjuus makkoomaa
Ei tätä poikoo hellyys haittaa
Ko akkoja huhkii laiasta laitaan)",

	uR"(Salivili hipput tupput täppyt äppyt tipput hilijalleen)",

	uR"(Sen minä sanon jotta purra pittää ei mua niin voan nielasta
Suat männä ite vaikka lännestä ittään vaan minä en luovu Ievasta
Sillä ei tätä poikoo kainous haittaa
Sillon ko tanssii laiasta laitaan)"
};

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
