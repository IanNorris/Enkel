#pragma once

void InitializeStdioVolumes();

void InsertInput(char input);
void ClearInput();
size_t ReadInputNoBlocking(char* buffer, size_t size);
size_t ReadInputBlocking(char* buffer, size_t size);
