; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    ; malloc(sizeof(string_proc_list)) = malloc(16)
    mov rdi, 16      
    call malloc       
    test rax, rax      
    je .error           

    ; rax apunta a la estructura que se creo recien
    ; Inicializar list->first = NULL
    mov qword [rax], 0
    mov qword [rax + 8], 0 ; Inicializar list->last = NULL
    ret

.error:
    mov rax, 0
    ret

string_proc_node_create_asm:
    ; rdi 
    ; rsi = puntero a hash

    test rsi, rsi     ; ver si hash == NULL
    je .error

    ;malloc(sizeof(string_proc_node)) = malloc(32)
    mov rdi, 32
    call malloc
    test rax, rax
    je .error

    ; rax tiene el puntero al nodo
    ; Inicializamos campos
    mov qword [rax], 0         ; next
    mov qword [rax + 8], 0     ; previous
    mov qword [rax + 16], rsi  ; hash (solo apuntar)
    mov byte [rax + 24], dil   ; type (dil = parte baja de rdi)

    ; devuelvo el puntero al nodo en rax
    ret

.error:
    mov rax, 0
    ret

string_proc_list_add_node_asm:
    ; rdi = lista (puntero a string_proc_list)
    ; sil = tipo
    ; rdx = hash

    mov rcx, rdi ; guardo lista en rcx 

    ; call a string_proc_node_create_asm
    movzx edi, sil    
    mov rsi, rdx    
    call string_proc_node_create_asm

    ; veo si devuelve NULL
    test rax, rax
    je .fin

    mov r8, rax

    ; veo si lista->first == NULL
    cmp qword [rcx], 0
    je .lista_vacia

    ; Si no está vacia lista->last->next = nodo
    mov rdx, [rcx + 8]       ; rdx = lista->last
    mov [rdx], r8            ; last->next = nodo
    mov [r8 + 8], rdx        ; nodo->previous = last
    mov [rcx + 8], r8        ; lista->last = nodo
    jmp .fin

.lista_vacia:
    ; lista->first = nodo
    ; lista->last  = nodo
    mov [rcx], r8
    mov [rcx + 8], r8

.fin:
    ret

string_proc_list_concat_asm:
    ; rdi = list
    ; sil = type
    ; rdx = hash

    test rdi, rdi
    je .return_null

    mov r13, [rdi]     
    mov r12, rdx     
    mov r15, rdx         
    movzx r8, sil        

.loop:
    test r13, r13
    je .done

    movzx r9, byte [r13 + 24]   ; r9 = node->type
    cmp r9, r8
    jne .next


    mov rsi, [r13 + 16]       
    mov rdi, r12               
    call str_concat        


    cmp r12, r15
    je .skip_free

    ; free(acumulado)
    mov rdi, r12
    call free

.skip_free:
    mov r12, rax                ; acumulado = result

.next:
    mov r13, [r13]              ; node = node->next
    jmp .loop

.done:
    mov rax, r12
    ret

.return_null:
    xor rax, rax
    ret
