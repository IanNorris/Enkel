#pragma once

void InitializeStdioVolumes();

void InsertInput(uint8_t input, bool scancode);
void ClearInput(bool scancode);
size_t ReadInputNoBlocking(uint8_t* buffer, size_t size, bool scancode);
size_t ReadInputBlocking(uint8_t* buffer, size_t size, bool scancode);
