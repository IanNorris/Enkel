#include <c-efi.h>

CEfiStatus efi_main(CEfiHandle imageHandle, CEfiSystemTable* systemTable)
{
	systemTable->con_out->reset(systemTable->con_out, 0);

	systemTable->con_out->output_string(systemTable->con_out, L"Starting Enkel...\r\n1");



	systemTable->con_out->output_string(systemTable->con_out, L"2");

	systemTable->con_out->output_string(systemTable->con_out, L"3");

	while (1);

	return 0;
}
