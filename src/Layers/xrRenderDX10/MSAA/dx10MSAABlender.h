#ifndef	dx10MSAABlender_included
#define	dx10MSAABlender_included

#if !defined(USE_DX12)
class CBlender_msaa : public IBlender
{
public:
	virtual LPCSTR getComment() { return "INTERNAL: DX10 msaa blender"; }
	virtual BOOL canBeDetailed() { return FALSE; }
	virtual BOOL canBeLMAPped() { return FALSE; }

	virtual void Compile(CBlender_Compile& C);
};
#endif // !USE_DX12
#endif	//	dx10RainBlender_included
