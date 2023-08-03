#pragma once

void DisableInterrupts();
void EnableInterrupts();
void InitIDT();
bool IsDebuggerPresent();
void __attribute__((used, noinline)) DebuggerHook();
