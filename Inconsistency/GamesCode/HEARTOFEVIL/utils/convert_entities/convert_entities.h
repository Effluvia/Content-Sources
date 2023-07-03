#include "ChunkFile.h"
#include "bsplib.h"
#include "utldict.h"

class KeyValue
{
public:
	KeyValue(const char *key, const char *value);
	void SetKey(const char *key);
	void SetValue(const char *value);

	const char *m_key;
	const char *m_value;
	KeyValue *m_next;
};

class KeyValues
{
public:
	KeyValues();
	void AddKeyValue(const char *key, const char *value);
	void SetKeyValue(const char *key, const char *value);
	bool HasKeyValue(const char *key, const char *value);
	bool HasKey(const char *key);
	const char *ValueForKey(const char *key, const char *default = "");
	KeyValue *Find(const char *key);

	KeyValue *m_keyvalues;
};

class vmf_entity;

class vmf_chunk
{
public:
	vmf_chunk(const char *name);
	void AddKeyValue(const char *key, const char *value);
	void SetKeyValue(const char *key, const char *value);
	void SetKeyValue(KeyValue *kv, const char *value);
	bool HasKeyValue(const char *key, const char *value);
	bool HasKey(const char *key);
	void AddChild(vmf_chunk *child);
	vmf_chunk *LookupChild(char *name);
	vmf_chunk *LookupNext(char *name);
	void RemoveChild(vmf_chunk *child);
	void AddConnection(
		const char *output,
		const char *target,
		const char *input,
		const char *param,
		const char *delay,
#define ONLY_ONCE_FALSE -1
#define ONLY_ONCE_TRUE 1
		int onlyOnce);
	void DumpConnections();
	void Write(CChunkFile *pFile);
	const char *ValueForKey(const char *key, const char *default = "");

	static vmf_chunk s_root;
	const char *m_name; /* name of the chunk */
	KeyValues *m_keyvalues;
	vmf_chunk *m_children;
	vmf_chunk *m_next;
	bool m_ignore; // entity removed itself
};

class vmf_entity;

class Connection
{
public:
	Connection(
		const char *output,
		vmf_entity *target,
		const char *input,
		const char *param,
		const char *delay,
		int onlyOnce);

	const char *m_output;
	vmf_entity *m_entity;
	const char *m_input;
	const char *m_param;
	const char *m_delay;
	int m_onlyOnce;
};

class EntityConverter;
class TargetName;

class vmf_entity
{
public:
	vmf_entity(vmf_chunk *chunk);
	const char *ValueForKey(const char *key, const char *default = "");
	bool HasKey(const char *key);
	const char *Classname() { return ValueForKey("classname"); }
	bool IsClassname(const char *classname) { return !Q_stricmp(ValueForKey("classname"), classname); }
	void CreateConverter(const char *converterClassname = 0);
	void Convert();
	void AddConnection(
		vmf_entity *target,
		const char *param,
		const char *delay,
		int onlyOnce);
	void AddConnection(
		const char *output,
		vmf_entity *target,
		const char *input,
		const char *param,
		const char *delay,
		int onlyOnce);
	int FindConnection(vmf_entity *targetEnt, const char *input);
	void ChangeConnectionInput(vmf_entity *targetEnt, const char *oldInput, const char *newInput );
	const char *UniqueName();
#define NO_TARGET_NAME ((const char *)-1)
	vmf_entity *AddEntity(const char *classname, const char *targetname = NULL, const char *converterClassname = NULL);
	vmf_entity *AddEnvMessage(const char *message);
	vmf_entity *AddEnvTaskText(void);
	vmf_entity *AddTriggerSoundscape(void);
	static vmf_entity *CreateDummy(const char *classname, const char *targetname);
	void ReplaceTextures(const char *textures[]);

	vmf_chunk *m_chunk;
	vmf_entity *m_next;
	CUtlVector< vmf_entity * > m_callers;
	TargetName *m_TargetName;
	bool m_bReferredToByOthers;
	EntityConverter *m_converter;
	CUtlVector< Connection > m_connections;
};

// TargetName class holds a list of all entities that share the same "targetname".
class TargetName
{
public:
	TargetName(const char *name);
	void AddEntity(vmf_entity *entity);
	bool IsSharedByDifferentClasses();
	const char *GetName() const;
	typedef CUtlVector<vmf_entity *> EntityListType;
	const TargetName::EntityListType &GetEntityList() const;
	const char *UniqueNameForEntity(vmf_entity *entity);
	EntityListType m_EntityList;
private:
	bool m_bIsSharedByDifferentClasses;
	char m_sTargetName[128];
	static char s_sUniqueName[128];
};

// This next stuff is based on BEGIN_DATADESC() etc in datamap.h
typedef enum {
	FIELD_VOID = 0,
	FIELD_ADD,			// add a new key and value
	FIELD_KEEP,			/* keep the key and value */
	FIELD_FUNC,			/* convert the key and/or value by calling a method */
	FIELD_REMOVE,		/* remove the key and value */
	FIELD_RENAME,		/* rename the key, keep the value */
	FIELD_CLASSNAME,	/* value is a classname to be converted */
	FIELD_ENTITY,		/* value is an entity name */
	FIELD_SET,			/* add keyvalue or replace existing value */
} converter_fieldtype_t;

class EntityConverter;

typedef void (EntityConverter::*converter_func_t)(KeyValue &kv);

struct converter_typedescription_t
{
	converter_fieldtype_t fieldType;
	const char *key;
	const char *rename;
	converter_func_t func;
};

struct converter_datamap_t
{
	converter_typedescription_t *dataDesc;
	int dataNumFields;
	char const *dataClassName;
	converter_datamap_t *baseMap;

	const char *convertedClassname;
	const char *offInput;
	const char *onInput;
	const char *toggleInput;
	const char *bestOutput;
	const char *masterInput;
	bool origin;
};

#define DECLARE_SIMPLE_CONVERTER() \
	static converter_datamap_t m_DataMap; \
	static converter_datamap_t *GetBaseMap(); \
	template <typename T> friend void DataMapAccess(T *, converter_datamap_t **p); \
	template <typename T> friend converter_datamap_t *DataMapInit(T *);

#define	DECLARE_CONVERTER_NO_BASE() \
	DECLARE_SIMPLE_CONVERTER() \
	virtual converter_datamap_t *GetDataDescMap( void );

#define	DECLARE_CONVERTER(baseClass) \
	typedef baseClass BaseClass; \
	DECLARE_CONVERTER_NO_BASE()

#define BEGIN_CONVERTER( className ) \
	converter_datamap_t className::m_DataMap = { 0, 0, #className, NULL }; \
	converter_datamap_t *className::GetDataDescMap( void ) { return &m_DataMap; } \
	converter_datamap_t *className::GetBaseMap() { converter_datamap_t *pResult; DataMapAccess((BaseClass *)NULL, &pResult); return pResult; } \
	BEGIN_CONVERTER_GUTS( className )

#define BEGIN_CONVERTER_NO_BASE( className ) \
	converter_datamap_t className::m_DataMap = { 0, 0, #className, NULL }; \
	converter_datamap_t *className::GetDataDescMap( void ) { return &m_DataMap; } \
	converter_datamap_t *className::GetBaseMap() { return NULL; } \
	BEGIN_CONVERTER_GUTS( className )

#define BEGIN_CONVERTER_GUTS( className ) \
	static EntityConverter *className##Factory( void )						\
	{																		\
		return static_cast< EntityConverter * >( new className );			\
	};																		\
	template <typename T> converter_datamap_t *DataMapInit(T *); \
	template <> converter_datamap_t *DataMapInit<className>( className * ); \
	namespace className##_DataDescInit \
	{ \
		converter_datamap_t *g_DataMapHolder = DataMapInit( (className *)NULL ); /* This can/will be used for some clean up duties later */ \
	} \
	\
	template <> converter_datamap_t *DataMapInit<className>( className * ) \
	{ \
		EntityConverter::s_factory.Insert(#className + 3, &className##Factory); \
		typedef className classNameTypedef; \
		converter_datamap_t &datamap = className::m_DataMap; \
		/*static CDatadescGeneratedNameHolder nameHolder(#className);*/ \
		datamap.dataClassName = #className + 3; /* skip "EC_" prefix */ \
		datamap.baseMap = className::GetBaseMap(); \
		datamap.convertedClassname = 0; \
		datamap.offInput = 0; \
		datamap.onInput = 0; \
		datamap.toggleInput = 0; \
		datamap.bestOutput = 0; \
		datamap.masterInput = 0;

#define BEGIN_KEYVALUES() \
		static converter_typedescription_t dataDesc[] = \
		{ \
		{ FIELD_VOID,0,0,0}, /* so you can define "empty" tables */

#define END_CONVERTER() \
		}; \
		\
		if ( sizeof( dataDesc ) > sizeof( dataDesc[0] ) ) \
		{ \
			classNameTypedef::m_DataMap.dataNumFields = ARRAYSIZE( dataDesc ) - 1; \
			classNameTypedef::m_DataMap.dataDesc 	  = &dataDesc[1]; \
		} \
		else \
		{ \
			classNameTypedef::m_DataMap.dataNumFields = 1; \
			classNameTypedef::m_DataMap.dataDesc 	  = dataDesc; \
		} \
		return &classNameTypedef::m_DataMap; \
	}

template <typename T> 
inline void DataMapAccess(T *ignored, converter_datamap_t **p)
{
	*p = &T::m_DataMap;
}

#define CONVERTED_CLASSNAME(name) \
	datamap.convertedClassname = #name; \
	EntityConverter::s_ConvertedClassName.Insert(datamap.dataClassName, #name);
#define OFF_INPUT(input) datamap.offInput = #input;
#define ON_INPUT(input) datamap.onInput = #input;
#define TOGGLE_INPUT(input) datamap.toggleInput = #input;
#define BEST_OUTPUT(output) datamap.bestOutput = #output;
#define MASTER_INPUT(input) datamap.masterInput = #input;

#define ADD_KEYVALUE(key, value) { FIELD_ADD, #key, #value, 0 }
#define KEEP_KEYVALUE(key) { FIELD_KEEP, #key, 0, 0 }
#define REMOVE_KEYVALUE(key) { FIELD_REMOVE, #key, 0, 0 }
#define RENAME_KEY(key,newName) { FIELD_RENAME, #key, #newName, 0 }
#define SET_KEYVALUE(key,value) { FIELD_SET, #key, #value, 0 }
#define XFORM_KEYVALUE(key,func) { FIELD_FUNC, #key, 0, static_cast <converter_func_t> (&classNameTypedef::func) }
#define CLASSNAME_VALUE(key) { FIELD_CLASSNAME, #key, #key, 0 }
#define CLASSNAME_KEYVALUE(key,newName) { FIELD_CLASSNAME, #key, #newName, 0 }
#define ENTITY_VALUE(key) { FIELD_ENTITY, #key, #key, 0 }

class EntityConverter
{
public:
	DECLARE_CONVERTER_NO_BASE()

	virtual void Init(vmf_entity *entity);
	virtual void Convert(void);
	virtual void Finalize(void);
	void AddKeyValue(KeyValue &kv);
	void AddKeyValue(const char *key, const char *value);
	void SetKeyValue(const char *key, const char *value);

	// Spawnflags convenience
	void SetSpawnflags(int sf);
	void UnsetSpawnflags(int sf);

	// keyvalue handlers
	virtual void Handle_unknown(KeyValue &kv);
//	virtual void Handle_classname(KeyValue &kv);
	virtual void Handle_killtarget(KeyValue &kv);
	virtual void Handle_master(KeyValue &kv);
	virtual void Handle_rendercolor(KeyValue &kv);
	virtual void Handle_spawnflags(KeyValue &kv);
	virtual void Handle_target(KeyValue &kv);
	virtual void Handle_targetname(KeyValue &kv);

	vmf_entity *m_entity;
	KeyValues *m_keyvalues; // the original keyvalue list
	bool m_converted;

	virtual const char *OffInput(vmf_entity *entity);
	virtual const char *OnInput(vmf_entity *entity);
	virtual const char *ToggleInput(vmf_entity *entity);
	virtual const char *BestOutput(vmf_entity *entity);
	virtual const char *MasterInput(vmf_entity *entity);
	virtual const char *ConvertedClassname(void);

	virtual bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);

	KeyValues &OriginalKeyValues(void);
	static KeyValues &OriginalKeyValues(vmf_entity *entity);

	static const char *ConvertedClassname(const char *classname);
	static EntityConverter *CreateConverterByName(const char *classname);

	static CUtlDict< const char *, int > s_ConvertedClassName;
	typedef EntityConverter *(*EntityFactoryFn)( void );
	static CUtlDict< EntityFactoryFn, int > s_factory;
};

class EC_ambient_generic : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_spawnflags(KeyValue &kv);
};

class EC_ambient_mp3 : public EC_ambient_generic
{
public:
	DECLARE_CONVERTER(EC_ambient_generic)
	void Handle_spawnflags(KeyValue &kv);
};

class EC_env_beam : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_beverage : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_bush : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_explosion : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_fade : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_fog : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_global : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	const char *OnInput( vmf_entity *entity );
	const char *OffInput( vmf_entity *entity );
	const char *ToggleInput( vmf_entity *entity );
};

class EC_env_message : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
	void Handle_message(KeyValue &kv);

	static const char *LetterName(const char *message);
};

class EC_env_palm : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_render : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert( void );
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_env_shake : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_shooter : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_env_sound : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	void Handle_roomtype(KeyValue &kv);
};

class EC_env_spark : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_spawnflags(KeyValue &kv);
};

class EC_env_sprite : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_rendercolor(KeyValue &kv);
};

class EC_func_breakable : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_explodemagnitude(KeyValue &kv);
};

class EC_func_button : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	void Convert(void);
	void Handle_spawnflags(KeyValue &kv);

	const char *m_szLetterName;
};

class EC_func_door : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
	void Handle_spawnflags(KeyValue &kv);
	void Handle_netname(KeyValue &kv);
	void Handle_locked_sound(KeyValue &kv);
	void Handle_movesnd(KeyValue &kv);
	void Handle_stopsnd(KeyValue &kv);
	void Handle_target(KeyValue &kv);
	void Handle_unlocked_sound(KeyValue &kv);
};

class EC_func_door_rotating : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
	void Finalize(void);
	void Handle_spawnflags(KeyValue &kv);
	void Handle_netname(KeyValue &kv);
	void Handle_locked_sound(KeyValue &kv);
	void Handle_movesnd(KeyValue &kv);
	void Handle_stopsnd(KeyValue &kv);
	void Handle_unlocked_sound(KeyValue &kv);
};

class EC_func_illusionary : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_func_ladder : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
#if 1
	void Convert(void);
#else
	void Init(vmf_entity *entity);
#endif
};

class EC_func_monsterclip : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
};

class EC_func_mortar_field : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_func_pendulum : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
};

class EC_func_rotating : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_sounds(KeyValue &kv);
};

class EC_func_rot_button : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	void Handle_spawnflags(KeyValue &kv);
};

class EC_func_tank : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_bullet(KeyValue &kv);
	void Handle_spawnflags(KeyValue &kv);
};

class EC_func_tankcontrols : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_target(KeyValue &kv);
};

class EC_func_tracktrain : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
	void Handle_spawnflags(KeyValue &kv);
};

class EC_func_train : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_movesnd(KeyValue &kv);
	void Handle_stopsnd(KeyValue &kv);
};

class EC_func_wall : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
//	void Convert();
};

class EC_func_wall_toggle : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};
class EC_func_water : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_infodecal : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert(void);
	void Replace(const char *texture, const char *oldName, const char *newName);
};

class EC_light : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_monstermaker : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
//	void Handle_monstertype(KeyValue &kv);
	void Handle_spawnflags(KeyValue &kv);
	void Handle_TriggerTarget(KeyValue &kv);
};

class EC_monster_xyz : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_TriggerTarget(KeyValue &kv);
};

class EC_monster_alien_controller : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_alien_slave : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_barnacle : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_barney : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_barneyzombie : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_bullchicken : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_charlie : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_charlie_dead : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_chumtoad : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_furniture : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_generic : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_gorilla : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_headcrab : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_huey : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Init(vmf_entity *entity);
	void Handle_spawnflags(KeyValue &kv);
	static bool ConvertPathCornerToPathTrack(const char *hueyName, const char *pathName);
};

class EC_monster_human_grunt : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_hgrunt_dead : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_grunt_repel : public EC_monster_human_grunt
{
public:
	DECLARE_CONVERTER(EC_monster_human_grunt)
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_monster_human_grunt_medic : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_grunt_medic_repel : public EC_monster_human_grunt_medic
{
public:
	DECLARE_CONVERTER(EC_monster_human_grunt_medic)
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_monster_houndeye : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_kophyaeger : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_kophyaeger_adult : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_kurtz : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_mikeforce : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_mikeforce_dead : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_mikeforce_repel : public EC_monster_mikeforce
{
public:
	DECLARE_CONVERTER(EC_monster_mikeforce)
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_monster_mikeforce_medic : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_mikeforce_medic_repel : public EC_monster_mikeforce_medic
{
public:
	DECLARE_CONVERTER(EC_monster_mikeforce_medic)
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_monster_peasant : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_peasant_dead : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_scientist_dead : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_sog : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_superzombie : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
	void Handle_weapons(KeyValue &kv);
};

class EC_monster_tripmine : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_vortigaunt : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
};

class EC_monster_zombie : public EC_monster_xyz
{
public:
	DECLARE_CONVERTER(EC_monster_xyz)
	void Handle_spawnflags(KeyValue &kv);
};

class EC_multi_manager : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_unknown(KeyValue &kv);
};

class EC_multisource : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
};

class EC_path_corner : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_message(KeyValue &kv);
};

class EC_path_track : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_message(KeyValue &kv);
};

class EC_player_loadsaved : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
//	void Handle_message(KeyValue &kv);
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);

	vmf_entity *m_env_message;
};

class EC_player_weaponstrip : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_scripted_sentence : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_scripted_sequence : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	void Convert(void);
	void Finalize(void);
	void Handle_target(KeyValue &kv);

	void CallBeginSequenceFromLogicAuto(void);

	bool m_bLoopWithTriggerRelay;
	bool m_bLoopWithSelf;
	bool m_bStartOnSpawn; // Possess the entity on spawn
	bool m_bWaitForInput; // Wait for BeginSequence input to play action sequence
	bool m_bTaskEnableScript; // is TASK_ENABLE_SCRIPT called in HL1

	vmf_entity * m_nextSS; // Scripted sequence to call after this one ends
	vmf_entity *m_loopEntity;
	static vmf_entity *s_logicAuto;
};

class EC_trigger_auto : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_target(KeyValue &kv);
};

class EC_trigger_camera : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_target(KeyValue &kv);
};

class EC_trigger_changelevel : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_changetarget(KeyValue &kv);
	void Handle_landmark(KeyValue &kv);
};

class EC_trigger_changetarget : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_trigger_endsection : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Convert( void );
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
};

class EC_trigger_hurt : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_target(KeyValue &kv);
	void Handle_spawnflags(KeyValue &kv);
};

class EC_trigger_multiple : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	void Finalize(void);
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);

	void Handle_message(KeyValue &kv);
	void Handle_spawnflags(KeyValue &kv);

#define HUD_USEABLE 1
	vmf_entity *m_useableEnt1;
	vmf_entity *m_useableEnt2;
	const char *m_message;
};

class EC_trigger_once : public EC_trigger_multiple
{
public:
	DECLARE_CONVERTER(EC_trigger_multiple)
};

class EC_trigger_push : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_speed(KeyValue &kv);
	void Handle_spawnflags(KeyValue &kv);
};

class EC_trigger_relay : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Init(vmf_entity *entity);
	bool AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce);
	void Handle_target(KeyValue &kv);

	bool m_bLoopWithScriptedSequence;
	int m_triggerstate;
};

class EC_trigger_teleport : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
};

class EC_xen_plantlight : public EntityConverter
{
public:
	DECLARE_CONVERTER(EntityConverter)
	void Handle_target(KeyValue &kv);
};

class CallersFixup
{
public:
	void Init(vmf_entity *entity);
	virtual void Fixup() = 0;

	vmf_entity *m_entity;
	CallersFixup *m_next;

	static void FixupAll();
	static CallersFixup *s_first;
};

class CallersFixup_multisource : public CallersFixup
{
public:
	void Fixup();
};

class CallersFixup_player_loadsaved : public CallersFixup
{
public:
	void Fixup();
	vmf_entity *m_env_message;
	const char *m_messagetime;
};

#define LTN_MSG 0x01
#define LTN_EXIT 0x02
#define LTN_REQUIRED (LTN_MSG | LTN_EXIT)
TargetName *LookupTargetName(const char *targetname, int flags = LTN_REQUIRED);
TargetName *LookupTargetNameFailOK(const char *targetname);
vmf_entity *FindEntity(vmf_entity *first, const char *name);
vmf_entity *FindEntityByClass(vmf_entity *first, const char *name);

inline const char *AllocStr(const char *s)
{
	return strcpy(new char [strlen(s) + 1], s);
};

bool Rotate180(vmf_entity *entity);
bool OriginBrushToKeyvalue(vmf_entity *entity);
void BrushToPointEntity(vmf_entity *entity);

void CEMsg(const char *fmt, ...);

struct vector
{
	float x, y, z;
};

bool GetBoundsOfSolid(vmf_chunk *solid, vector &min, vector &max);
vmf_entity *FindEnclosedEntity(vmf_entity *outer, vmf_entity *first, float fudge);

extern vmf_entity *g_entities;