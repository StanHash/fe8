
#pragma once

#include "global.h"

struct Proc;

typedef void* ProcPtr;
typedef void(*ProcFunc)(ProcPtr proc);

struct ProcScr
{
    short cmd;
    short imm;
    void const* ptr;
};

enum
{
    PROC_CMD_END,
    PROC_CMD_NAME,
    PROC_CMD_CALL,
    PROC_CMD_REPEAT,
    PROC_CMD_ONEND,
    PROC_CMD_START_CHILD,
    PROC_CMD_START_CHILD_BLOCKING,
    PROC_CMD_START_BUGGED,
    PROC_CMD_WHILE_EXISTS,
    PROC_CMD_END_EACH,
    PROC_CMD_BREAK_EACH,
    PROC_CMD_LABEL,
    PROC_CMD_GOTO,
    PROC_CMD_GOTO_SCR,
    PROC_CMD_SLEEP,
    PROC_CMD_MARK,
    PROC_CMD_BLOCK,
    PROC_CMD_END_IF_DUP,
    PROC_CMD_SET_FLAG2,
    PROC_CMD_13,
    PROC_CMD_WHILE,
    PROC_CMD_15,
    PROC_CMD_CALL_2,
    PROC_CMD_END_DUPS,
    PROC_CMD_CALL_ARG,
    PROC_CMD_19,
};

#define PROC_END                          { PROC_CMD_END, 0, 0 }
#define PROC_NAME(nameStr)                { PROC_CMD_NAME, 0, (nameStr) }
#define PROC_CALL(func)                   { PROC_CMD_CALL, 0, (func) }
#define PROC_REPEAT(func)                 { PROC_CMD_REPEAT, 0, (func) }
#define PROC_ONEND(func)                  { PROC_CMD_ONEND, 0, (func) }
#define PROC_START_CHILD(procScr)         { PROC_CMD_START_CHILD, 0, (procScr) }
#define PROC_START_CHILD_LOCKING(procScr) { PROC_CMD_START_CHILD_BLOCKING, 1, (procScr) }
#define PROC_START_BUGGED(procScr)        { PROC_CMD_START_BUGGED, 0, (procScr) }
#define PROC_WHILE_EXISTS(procScr)        { PROC_CMD_WHILE_EXISTS, 0, (procScr) }
#define PROC_END_EACH(procScr)            { PROC_CMD_END_EACH, 0, (procScr) }
#define PROC_BREAK_EACH(procScr)          { PROC_CMD_BREAK_EACH, 0, (procScr) }
#define PROC_LABEL(label)                 { PROC_CMD_LABEL, (label), 0 }
#define PROC_GOTO(label)                  { PROC_CMD_GOTO, (label), 0 }
#define PROC_GOTO_SCR(procScr)            { PROC_CMD_GOTO_SCR, 0, (procScr) }
#define PROC_SLEEP(duration)              { PROC_CMD_SLEEP, (duration), 0 }
#define PROC_MARK(mark)                   { PROC_CMD_MARK, (mark), 0 }
#define PROC_BLOCK                        { PROC_CMD_BLOCK, 0, 0 }
#define PROC_END_IF_DUP                   { PROC_CMD_END_IF_DUP, 0, 0 }
#define PROC_SET_FLAG2                    { PROC_CMD_SET_FLAG2, 0, 0 }
#define PROC_13                           { PROC_CMD_13, 0, 0 }
#define PROC_WHILE(func)                  { PROC_CMD_WHILE, 0, (func) }
#define PROC_15                           { PROC_CMD_15, 0, 0 }
#define PROC_CALL_2(func)                 { PROC_CMD_CALL_2, 0, (func) }
#define PROC_END_DUPS                     { PROC_CMD_END_DUPS, 0, 0 }
#define PROC_CALL_ARG(func, arg)          { PROC_CMD_CALL_ARG, (arg), (func) }
#define PROC_19                           { PROC_CMD_19, 0, 0 }

// allows local Proc structs to invoke the general Proc
// fields when creating local Proc definitions.
#define PROC_HEADER                                                                        \
    struct ProcScr const* proc_script; /* pointer to proc script */                        \
    struct ProcScr const* proc_scrCur; /* pointer to currently executing script command */ \
    ProcFunc proc_endFunc; /* callback to run upon delegint the process */                 \
    ProcFunc proc_repeatFunc; /* callback to run once each frame. */                       \
                          /* disables script execution when not null */                    \
    char const* proc_name;                                                                 \
    ProcPtr proc_parent; /* pointer to parent proc. If this proc is a root proc, */        \
                         /* this member is an integer which is the root index. */          \
    ProcPtr proc_child; /* pointer to most recently added child */                         \
    ProcPtr proc_next; /* next sibling */                                                  \
    ProcPtr proc_prev; /* previous sibling */                                              \
    s16 proc_sleepTime;                                                                    \
    u8 proc_mark;                                                                          \
    u8 proc_flags;                                                                         \
    u8 proc_lockCnt  /* wait semaphore. Process execution */                               \
                     /* is blocked when this is nonzero. */                                \

// Proc struct for general useage.
struct Proc
{
    /* 00 */ PROC_HEADER;

    /* 2C */ int i32_2C;
    /* 30 */ int i32_30;
    /* 34 */ int i32_34;
    /* 38 */ int i32_38;
    /* 3C */ int i32_3C;
    /* 40 */ int i32_40;

    /* 44 */ u8 pad_38[0x4A - 0x44];

    /* 4A */ short i16_4A;
    /* 4C */ short i16_4C; 
    /* 4E */ short i16_4E;
    /* 50 */ short i16_50;
    /* 52 */ short i16_52;

    /* 54 */ void* ptr_54;
    /* 58 */ int i32_58;
    /* 5C */ int i32_5C;

    /* 60 */ u8 pad_60[0x64 - 0x60];

    /* 64 */ short i16_64;
    /* 66 */ short i16_66;
    /* 68 */ short i16_68;
    /* 6A */ short i16_6A;
};

struct ProcFindIterator
{
    /* 00 */ struct Proc* proc;
    /* 04 */ struct ProcScr const* script;
    /* 08 */ int count;
};

enum
{
    PROC_MARK_0 = 0x00,
    PROC_MARK_1 = 0x01,
    PROC_MARK_2 = 0x02,
    PROC_MARK_3 = 0x03,
    PROC_MARK_4 = 0x04,
    PROC_MARK_5 = 0x05,
    PROC_MARK_6 = 0x06,
    PROC_MARK_7 = 0x07,

    PROC_MARK_B = 0x0B,
};

#define PROC_TREE_VSYNC ((ProcPtr) 0)
#define PROC_TREE_1     ((ProcPtr) 1)
#define PROC_TREE_2     ((ProcPtr) 2)
#define PROC_TREE_3     ((ProcPtr) 3)
#define PROC_TREE_4     ((ProcPtr) 4)
#define PROC_TREE_5     ((ProcPtr) 5)
#define PROC_TREE_6     ((ProcPtr) 6)
#define PROC_TREE_7     ((ProcPtr) 7)

#define ROOT_PROC(treenum) (*(gProcTreeRootArray + (treenum)))

extern ProcPtr gProcTreeRootArray[8];

void InitProcs(void);
ProcPtr SpawnProc(struct ProcScr const* script, ProcPtr parent);
ProcPtr SpawnProcLocking(struct ProcScr const* script, ProcPtr parent);
void Proc_End(ProcPtr proc);
void Proc_Run(ProcPtr proc);
void Proc_Break(ProcPtr proc);
ProcPtr FindProc(struct ProcScr const* script);
void Proc_Goto(ProcPtr proc, int label);
void Proc_GotoScript(ProcPtr proc, const struct ProcScr* script);
void Proc_SetMark(ProcPtr proc, int mark);
void Proc_SetEndCb(ProcPtr proc, ProcFunc func);
void ForEveryProc(ProcFunc func);
void ForEachProc(struct ProcScr const* script, ProcFunc func);
void ForEachMarkedProc(int mark, ProcFunc func);
void LockEachMarkedProc(int mark);
void ReleaseEachMarkedProc(int mark);
void EndEachMarkedProc(int mark);
void EndEachProc(struct ProcScr const* script);
void BreakEachProc(struct ProcScr const* script);
void Proc_SetRepeatFunc(ProcPtr proc, ProcFunc func);
void Proc_Lock(ProcPtr proc);
void Proc_Release(ProcPtr proc);
// ??? Proc_FindAfter(???);
// ??? Proc_FindAfterWithParent(???);
// ??? sub_80034D4(???);
// ??? sub_80034FC(???);
void BeginFindProc(struct ProcFindIterator* it, struct ProcScr const* script);
ProcPtr NextFindProc(struct ProcFindIterator* it);
