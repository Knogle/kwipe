; rc4_genrand_4096_to_buf_avx2.asm
; Generiert 4096 Bytes pseudorandom Daten mit RC4, optimiert mit AVX2

; Funktionsprototyp:
; void rc4_genrand_4096_to_buf_avx2(rc4_state_t* state, unsigned char* bufpos);

section .text
    global rc4_genrand_4096_to_buf_avx2

rc4_genrand_4096_to_buf_avx2:
    ; Funktionsparameter:
    ; rc4_state_t* state    -> RDI
    ; unsigned char* bufpos -> RSI

    push    rbp                 ; Prolog
    mov     rbp, rsp
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15

    ; Lokale Variablen und Registerzuweisungen
    ; rdi -> state (rc4_state_t*)
    ; rsi -> bufpos (unsigned char*)
    ; rcx, rdx, r8, r9 werden als temporäre Register verwendet

    ; Setze n = 0 (Zählvariable für die Schleife)
    xor     rax, rax            ; rax = 0
    mov     r13, rax            ; r13 = n = 0

    ; OUTPUT_DATA_LENGTH = 4096
    mov     r12, 4096           ; r12 = OUTPUT_DATA_LENGTH

.loop_start:
    cmp     r13, r12            ; Vergleiche n mit OUTPUT_DATA_LENGTH
    jge     .loop_end           ; Wenn n >= OUTPUT_DATA_LENGTH, Schleife beenden

    ; Prefetch der S-Box-Daten
    mov     rax, [rdi]          ; Lade state->S (Zeiger auf S-Box) in rax
    movzx   ebx, byte [rdi + 256]   ; Lade state->i in ebx
    add     ebx, 16             ; ebx = state->i + 16
    and     ebx, 0xFF           ; Stelle sicher, dass Index innerhalb 0-255 bleibt
    lea     rcx, [rax + rbx]    ; Adresse von state->S[state->i + 16]
    prefetcht0 [rcx]            ; Prefetch der Daten

    ; Inkrementiere den Counter (CTR-Modus)
    ; state->counter++
    mov     rax, [rdi + 264]    ; Lade state->counter
    inc     rax                 ; Inkrementiere counter
    mov     [rdi + 264], rax    ; Speichere counter zurück

    ; Mixe den Counter in die S-Box
    ; uint64_t counter_value = state->counter;
    mov     r14, rax            ; r14 = counter_value

    mov     r15, 0              ; Schleifenzähler i = 0

.counter_mix_loop:
    cmp     r15, 8              ; Vergleiche i mit 8
    jge     .generate_random    ; Wenn i >= 8, verlasse die Schleife

    ; state->i = (state->i + 1) % 256;
    movzx   ebx, byte [rdi + 256]   ; Lade state->i
    inc     ebx                 ; state->i + 1
    and     ebx, 0xFF           ; Modulo 256
    mov     byte [rdi + 256], bl    ; Speichere state->i

    ; state->j = (state->j + state->S[state->i] + (counter_value & 0xFF)) % 256;
    movzx   ecx, byte [rdi + 257]   ; Lade state->j
    mov     rax, [rdi]          ; Lade Zeiger auf S-Box
    movzx   edx, byte [rax + rbx]   ; Lade state->S[state->i]
    movzx   edi, dl             ; edi = state->S[state->i]
    movzx   esi, byte r14b      ; (counter_value & 0xFF)
    add     ecx, edi            ; state->j + state->S[state->i]
    add     ecx, esi            ; + (counter_value & 0xFF)
    and     ecx, 0xFF           ; Modulo 256
    mov     byte [rdi + 257], cl    ; Speichere state->j

    ; Swap state->S[state->i] und state->S[state->j]
    movzx   edx, byte [rax + rbx]   ; Lade state->S[state->i]
    movzx   edi, byte [rax + rcx]   ; Lade state->S[state->j]
    mov     byte [rax + rbx], dil   ; state->S[state->i] = state->S[state->j]
    mov     byte [rax + rcx], dl    ; state->S[state->j] = temp

    shr     r14, 8              ; counter_value >>= 8
    inc     r15                 ; i++
    jmp     .counter_mix_loop   ; Nächste Iteration

.generate_random:
    ; Generiere 32 Bytes pseudorandom Daten
    mov     r15, 0              ; Schleifenzähler i = 0

.random_gen_loop:
    cmp     r15, 32             ; Vergleiche i mit 32
    jge     .store_data         ; Wenn i >= 32, verlasse die Schleife

    ; state->i = (state->i + 1) % 256;
    movzx   ebx, byte [rdi + 256]   ; Lade state->i
    inc     ebx                 ; state->i + 1
    and     ebx, 0xFF           ; Modulo 256
    mov     byte [rdi + 256], bl    ; Speichere state->i

    ; state->j = (state->j + state->S[state->i]) % 256;
    movzx   ecx, byte [rdi + 257]   ; Lade state->j
    mov     rax, [rdi]          ; Lade Zeiger auf S-Box
    movzx   edx, byte [rax + rbx]   ; Lade state->S[state->i]
    add     ecx, edx            ; state->j + state->S[state->i]
    and     ecx, 0xFF           ; Modulo 256
    mov     byte [rdi + 257], cl    ; Speichere state->j

    ; Swap state->S[state->i] und state->S[state->j]
    movzx   edx, byte [rax + rbx]   ; Lade state->S[state->i]
    movzx   edi, byte [rax + rcx]   ; Lade state->S[state->j]
    mov     byte [rax + rbx], dil   ; state->S[state->i] = state->S[state->j]
    mov     byte [rax + rcx], dl    ; state->S[state->j] = temp

    ; temp_buffer[n + i] = state->S[(state->S[state->i] + state->S[state->j]) % 256];
    movzx   edx, byte [rax + rbx]   ; state->S[state->i]
    movzx   edi, byte [rax + rcx]   ; state->S[state->j]
    add     edx, edi            ; Summe der beiden
    and     edx, 0xFF           ; Modulo 256
    movzx   edx, byte [rax + rdx]   ; Lade state->S[Summe]

    ; Speichere das Byte in den temporären Puffer
    mov     byte [rsp + r15], dl

    inc     r15                 ; i++
    jmp     .random_gen_loop    ; Nächste Iteration

.store_data:
    ; Lade die 32 Bytes aus dem temporären Puffer in ein AVX2-Register
    vmovdqu ymm0, [rsp]         ; Lade 32 Bytes in ymm0

    ; Speichere die 32 Bytes in den Ausgabepuffer bufpos
    vmovdqu [rsi + r13], ymm0

    ; Inkrementiere n um 32
    add     r13, 32
    jmp     .loop_start         ; Nächste Iteration der Hauptschleife

.loop_end:
    ; Epilog
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    pop     rbp
    ret
