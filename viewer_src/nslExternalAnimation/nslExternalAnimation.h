/** 
 * nslExternalAnimation.h    v2.0
 *
 *		Copyright (c) 2011, Fumi.Iseki
 * 
 */

#ifndef _NSL_EXTERNAL_ANIMATIONL_H__
#define _NSL_EXTERNAL_ANIMATIONL_H__



#define  EXANIM_CHANNEL_NUM		21
#define  EXANIM_DATA_SIZE		64

#define  EXANIM_JOINT_NUM		19

#define  EXANIM_ZERO_UUID		"00000000-0000-0000-0000-000000000000"

#define  EXANIM_INDEX_LEN		40
#define  EXANIM_UUID_LEN		36
#define	 EXANIM_INDEX_NAME		"AnimationIndex"




namespace  nsl {





class  ExternalAnimation
{
public:
	LLUUID		uuid;
	int			channel;

	BOOL		isExternal;

	void*		indexHandle;
	char*		ptrIndex;

	void*		mapHandle[EXANIM_JOINT_NUM	];
	double*		ptrShm[EXANIM_JOINT_NUM	];


public:
	ExternalAnimation()  {}
	ExternalAnimation(LLUUID id) { init(id);}
	~ExternalAnimation() { CloseShm();}

	void		init(LLUUID id);

	void		CreateAllJointsShm(void);
	void 		CreateJointShm(int n);
	void 		OpenJointShm(int n);
	void		CloseShm(void);

	void		CreateIndexShm(void);
	void		OpenIndexShm(void);
	void		CloseIndexShm(void);
	int			GetChannelIndexShm(std::string uuid);
	void 		CheckChannelIndexShm(void);


	LLQuaternion	GetAnimationRotation(std::string joint_name);
	LLVector3		GetAnimationPosition(std::string joint_name);
	LLVector3		GetAnimationScale   (std::string joint_name);

};






}		// namespace



#endif		// _NSL_EXTERNAL_ANIMATIONL_H__
