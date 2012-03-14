#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
typedef uint32_t ARMword;
typedef int32_t ARMsword;
typedef uint64_t ARMdword;
typedef int64_t ARMsdword;
#else
typedef unsigned int ARMword;	/* must be 32 bits wide */
typedef signed int ARMsword;
typedef unsigned long long ARMdword;	/* Must be at least 64 bits wide.  */
typedef signed long long ARMsdword;
#endif
typedef struct ARMul_State ARMul_State;

typedef unsigned ARMul_SWIhandler(ARMul_State * state, ARMword number);

extern __declspec(dllimport) void ARMul_EmulateInit (void);
extern __declspec(dllimport) ARMul_State *ARMul_NewState (void);
extern __declspec(dllimport) void ARMul_Reset (ARMul_State * state);
extern __declspec(dllimport) ARMword ARMul_DoProg (ARMul_State * state);
extern __declspec(dllimport) ARMword ARMul_DoInstr (ARMul_State * state);

extern void __declspec(dllimport) ARMul_MemoryInit2 (ARMul_State* state, void* memory, unsigned long initmemsize);
extern void __declspec(dllimport) ARMul_MemoryExit (ARMul_State* state);

extern void __declspec(dllimport) ARMul_SetSWIhandler (ARMul_State* state, ARMul_SWIhandler*, void* user);
extern void __declspec(dllimport) ARMul_SetMemErrHandler (ARMul_State* state, ARMul_SWIhandler*);

extern __declspec(dllimport) ARMword ARMul_GetReg (ARMul_State * state, unsigned mode,
	unsigned reg);
extern __declspec(dllimport) void ARMul_SetReg (ARMul_State * state, unsigned mode, unsigned reg,
	ARMword value);
extern __declspec(dllimport) ARMword ARMul_GetPC (ARMul_State * state);
extern __declspec(dllimport) ARMword ARMul_GetNextPC (ARMul_State * state);
extern __declspec(dllimport) void ARMul_SetPC (ARMul_State * state, ARMword value);

extern __declspec(dllimport) ARMword ARMul_GetCPSR (ARMul_State * state);

extern __declspec(dllimport) ARMword* ARMul_GetRegs (ARMul_State * state);

#ifdef __cplusplus
}
#endif
