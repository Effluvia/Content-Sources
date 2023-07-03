// convert_entities.cpp : Defines the entry point for the console application.
//

#include "convert_entities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int num_chunks = 0;
int g_num_entities = 0;

vmf_chunk vmf_chunk::s_root("");

vmf_entity *g_entities = 0, *g_last_entity = 0;

class TargetName;
CUtlDict< TargetName *, int > g_targetnames;

ChunkFileResult_t LoadChunkCallback(CChunkFile *pFile, void *pData, char const *pChunkName);
ChunkFileResult_t LoadKeyValueCallback(const char *szKey, const char *szValue, void *pData, char const *pChunkName);

char TargetName::s_sUniqueName[128];

void CEMsg(const char *pszFormat, ...)
{
	int     result;
	va_list arg_ptr;

	va_start(arg_ptr, pszFormat);
	result = vfprintf(stdout, pszFormat, arg_ptr);
	va_end(arg_ptr);
}


KeyValue::KeyValue(const char *key, const char *value)
{
	SetKey(key);
	SetValue(value);
	m_next = 0;
}

void KeyValue::SetKey(const char *key)
{
	m_key = AllocStr(key);
}

void KeyValue::SetValue(const char *value)
{
	m_value = AllocStr(value);
}

KeyValues::KeyValues()
{
	m_keyvalues = 0;
}

void KeyValues::AddKeyValue(const char *key, const char *value)
{
	KeyValue *kv = new KeyValue(key, value);

	if (m_keyvalues)
	{
		KeyValue *last = m_keyvalues;
		while (last->m_next)
			last = last->m_next;
		last->m_next = kv;
	}
	else
	{
		m_keyvalues = kv;
	}
}

void KeyValues::SetKeyValue(const char *key, const char *value)
{
	KeyValue *kv = Find(key);
	if (kv)
		kv->SetValue(value);
	else
		AddKeyValue(key, value);
}

bool KeyValues::HasKeyValue(const char *key, const char *value)
{
	KeyValue *kv;
	for (kv = m_keyvalues; kv; kv = kv->m_next)
	{
		if (!Q_strcmp(kv->m_key, key) && !Q_strcmp(kv->m_value, value))
			return true;
	}
	return false;
}

bool KeyValues::HasKey(const char *key)
{
	return Find(key) != NULL;
}

const char *KeyValues::ValueForKey(const char *key, const char *default)
{
	KeyValue *kv = Find(key);
	return kv ? kv->m_value : default;
}

KeyValue *KeyValues::Find(const char *key)
{
	KeyValue *kv;
	for (kv = m_keyvalues; kv; kv = kv->m_next)
	{
		if (!Q_strcmp(kv->m_key, key))
			return kv;
	}
	return 0;
}

//-----------------------------------------------------------------------------
Connection::Connection(
	const char *output,
	vmf_entity *target,
	const char *input,
	const char *param,
	const char *delay,
	int onlyOnce)
{
	m_output = AllocStr(output);
	m_entity = target;
	m_input = AllocStr(input);
	m_param = AllocStr(param);
	m_delay = AllocStr(delay);
	m_onlyOnce = onlyOnce;
}

//-----------------------------------------------------------------------------
vmf_entity::vmf_entity(vmf_chunk *chunk)
{
	m_chunk = chunk;
	m_next = 0;
	m_TargetName = 0;
	m_bReferredToByOthers = false;
	m_converter = 0;
}

//-----------------------------------------------------------------------------
const char *vmf_entity::ValueForKey(const char *key, const char *default)
{
	return m_chunk->ValueForKey(key, default);
}

//-----------------------------------------------------------------------------
bool vmf_entity::HasKey(const char *key)
{
	return m_chunk->HasKey(key);
}

//-----------------------------------------------------------------------------
void vmf_entity::CreateConverter(const char *converterClassname)
{
	// AddEntity() is passed the converted classname
	const char *classname = converterClassname ? converterClassname : Classname();

#if 1
	m_converter = EntityConverter::CreateConverterByName(classname);
#else
	if (!Q_strcmp(classname, "ambient_generic"))
		m_converter = new EC_ambient_generic();

	else if (!Q_strcmp(classname, "env_beam"))
		m_converter = new EC_env_beam();

	else if (!Q_strcmp(classname, "env_beverage"))
		m_converter = new EC_env_beverage();

	else if (!Q_strcmp(classname, "env_explosion"))
		m_converter = new EC_env_explosion();

	else if (!Q_strcmp(classname, "env_fade"))
		m_converter = new EC_env_fade();

	else if (!Q_strcmp(classname, "env_fog"))
		m_converter = new EC_env_fog();

	else if (!Q_strcmp(classname, "env_global"))
		m_converter = new EC_env_global();

	else if (!Q_strcmp(classname, "env_message"))
		m_converter = new EC_env_message();

	else if (!Q_strcmp(classname, "env_palm"))
		m_converter = new EC_env_palm();

	else if (!Q_strcmp(classname, "env_render"))
		m_converter = new EC_env_render();

	else if (!Q_strcmp(classname, "env_shake"))
		m_converter = new EC_env_shake();

	else if (!Q_strcmp(classname, "env_shooter"))
		m_converter = new EC_env_shooter();

	else if (!Q_strcmp(classname, "env_sound"))
		m_converter = new EC_env_sound();

	else if (!Q_strcmp(classname, "env_sprite"))
		m_converter = new EC_env_sprite();

	else if (!Q_strcmp(classname, "func_breakable"))
		m_converter = new EC_func_breakable();

	else if (!Q_strcmp(classname, "func_button"))
		m_converter = new EC_func_button();

	else if (!Q_strcmp(classname, "func_door"))
		m_converter = new EC_func_door();

	else if (!Q_strcmp(classname, "func_door_rotating"))
		m_converter = new EC_func_door_rotating();

	else if (!Q_strcmp(classname, "func_illusionary"))
		m_converter = new EC_func_illusionary();

	else if (!Q_strcmp(classname, "func_ladder"))
		m_converter = new EC_func_ladder();

	else if (!Q_strcmp(classname, "func_monsterclip"))
		m_converter = new EC_func_monsterclip();

	else if (!Q_strcmp(classname, "func_pendulum"))
		m_converter = new EC_func_pendulum();

	else if (!Q_strcmp(classname, "func_rotating"))
		m_converter = new EC_func_rotating();

	else if (!Q_strcmp(classname, "func_rot_button"))
		m_converter = new EC_func_rot_button();

	else if (!Q_strcmp(classname, "func_tank"))
		m_converter = new EC_func_tank();

	else if (!Q_strcmp(classname, "func_tankcontrols"))
		m_converter = new EC_func_tankcontrols();

	else if (!Q_strcmp(classname, "func_tracktrain"))
		m_converter = new EC_func_tracktrain();

	else if (!Q_strcmp(classname, "func_train"))
		m_converter = new EC_func_train();

	else if (!Q_strcmp(classname, "func_wall"))
		m_converter = new EC_func_wall();

	else if (!Q_strcmp(classname, "func_wall_toggle"))
		m_converter = new EC_func_wall_toggle();

	else if (!Q_strcmp(classname, "infodecal"))
		m_converter = new EC_infodecal();

	else if (!Q_strcmp(classname, "light"))
		m_converter = new EC_light();

	else if (!Q_strcmp(classname, "monstermaker"))
		m_converter = new EC_monstermaker();

	else if (!Q_strcmp(classname, "monster_alien_controller"))
		m_converter = new EC_monster_alien_controller();

	else if (!Q_strcmp(classname, "monster_barney"))
		m_converter = new EC_monster_barney();

	else if (!Q_strcmp(classname, "monster_barneyzombie"))
		m_converter = new EC_monster_barneyzombie();

	else if (!Q_strcmp(classname, "monster_bullchicken"))
		m_converter = new EC_monster_bullchicken();

	else if (!Q_strcmp(classname, "monster_charlie"))
		m_converter = new EC_monster_charlie();

	else if (!Q_strcmp(classname, "monster_furniture"))
		m_converter = new EC_monster_furniture();

	else if (!Q_strcmp(classname, "monster_gorilla"))
		m_converter = new EC_monster_gorilla();

	else if (!Q_strcmp(classname, "monster_grunt_medic_repel"))
		m_converter = new EC_monster_grunt_medic_repel();

	else if (!Q_strcmp(classname, "monster_grunt_repel"))
		m_converter = new EC_monster_grunt_repel();

	else if (!Q_strcmp(classname, "monster_headcrab"))
		m_converter = new EC_monster_headcrab();

	else if (!Q_strcmp(classname, "monster_huey"))
		m_converter = new EC_monster_huey();

	else if (!Q_strcmp(classname, "monster_human_grunt"))
		m_converter = new EC_monster_human_grunt();

	else if (!Q_strcmp(classname, "monster_hgrunt_dead"))
		m_converter = new EC_monster_hgrunt_dead();

	else if (!Q_strcmp(classname, "monster_houndeye"))
		m_converter = new EC_monster_houndeye();

	else if (!Q_strcmp(classname, "monster_human_grunt_medic"))
		m_converter = new EC_monster_human_grunt_medic();

	else if (!Q_strcmp(classname, "monster_kurtz"))
		m_converter = new EC_monster_kurtz();

	else if (!Q_strcmp(classname, "monster_mikeforce"))
		m_converter = new EC_monster_mikeforce();

	else if (!Q_strcmp(classname, "monster_mikeforce_dead"))
		m_converter = new EC_monster_mikeforce_dead();

	else if (!Q_strcmp(classname, "monster_mikeforce_repel"))
		m_converter = new EC_monster_mikeforce_repel();

	else if (!Q_strcmp(classname, "monster_mikeforce_medic"))
		m_converter = new EC_monster_mikeforce_medic();

	else if (!Q_strcmp(classname, "monster_mikeforce_medic_repel"))
		m_converter = new EC_monster_mikeforce_medic_repel();

	else if (!Q_strcmp(classname, "monster_peasant"))
		m_converter = new EC_monster_peasant();

	else if (!Q_strcmp(classname, "monster_sog"))
		m_converter = new EC_monster_sog();

	else if (!Q_strcmp(classname, "monster_superzombie"))
		m_converter = new EC_monster_superzombie();

	else if (!Q_strcmp(classname, "monster_zombie"))
		m_converter = new EC_monster_zombie();

	else if (!Q_strncmp("monster_", classname, strlen("monster_")))
		m_converter = new EC_monster_xyz();

	else if (!Q_strcmp(classname, "multisource"))
		m_converter = new EC_multisource();

	else if (!Q_strcmp(classname, "multi_manager"))
		m_converter = new EC_multi_manager();

	else if (!Q_strcmp(classname, "path_corner"))
		m_converter = new EC_path_corner();

	else if (!Q_strcmp(classname, "path_track"))
		m_converter = new EC_path_track();

	else if (!Q_strcmp(classname, "player_loadsaved"))
		m_converter = new EC_player_loadsaved();

	else if (!Q_strcmp(classname, "scripted_sentence"))
		m_converter = new EC_scripted_sentence();

	else if (!Q_strcmp(classname, "scripted_sequence"))
		m_converter = new EC_scripted_sequence();

	else if (!Q_strcmp(classname, "trigger_auto"))
		m_converter = new EC_trigger_auto();

	else if (!Q_strcmp(classname, "trigger_camera"))
		m_converter = new EC_trigger_camera();

	else if (!Q_strcmp(classname, "trigger_changelevel"))
		m_converter = new EC_trigger_changelevel();

	else if (!Q_strcmp(classname, "trigger_changetarget"))
		m_converter = new EC_trigger_changetarget();

	else if (!Q_strcmp(classname, "trigger_hurt"))
		m_converter = new EC_trigger_hurt();

	else if (!Q_strcmp(classname, "trigger_multiple"))
		m_converter = new EC_trigger_multiple();

	else if (!Q_strcmp(classname, "trigger_once"))
		m_converter = new EC_trigger_once();

	else if (!Q_strcmp(classname, "trigger_push"))
		m_converter = new EC_trigger_push();

	else if (!Q_strcmp(classname, "trigger_relay"))
		m_converter = new EC_trigger_relay();

	else {
//		fprintf(stdout, "*** WARNING: using generic converter for %s\n", classname);
		m_converter = new EntityConverter();
	}
#endif

	m_converter->Init(this);
}

//-----------------------------------------------------------------------------
void vmf_entity::Convert()
{
	m_converter->Convert();
}

//-----------------------------------------------------------------------------
void vmf_entity::AddConnection(
		vmf_entity *target,
		const char *param,
		const char *delay,
		int onlyOnce)
{
	AddConnection(
		m_converter->BestOutput(target),
		target,
		target->m_converter->ToggleInput(this),
		param,
		delay,
		onlyOnce);
}

//-----------------------------------------------------------------------------
void vmf_entity::AddConnection(
		const char *output,
		vmf_entity *target,
		const char *input,
		const char *param,
		const char *delay,
		int onlyOnce)
{
	if (target->m_converter &&
		target->m_converter->AddConnection(this, output, input, param, delay, onlyOnce))
		return;

	m_connections.AddToTail( Connection(output, target, input, param, delay, onlyOnce) );
//	m_chunk->AddConnection(output, target->UniqueName(), input, param, delay, onlyOnce);
	target->m_callers.AddToTail(this);
	target->m_bReferredToByOthers = true;
}

//-----------------------------------------------------------------------------
int vmf_entity::FindConnection(vmf_entity *targetEnt, const char *input)
{
	for (int i = 0; i < m_connections.Count(); i++)
	{
		Connection &c = m_connections[i];
		if (c.m_entity == targetEnt && !Q_strcmp(c.m_input, input))
			return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
void vmf_entity::ChangeConnectionInput(vmf_entity *targetEnt, const char *oldInput, const char *newInput )
{
	int i = FindConnection(targetEnt, oldInput);
	if (i < 0)
	{
		fprintf(stdout, "ChangeConnectionInput: can't match %s %s\n", targetEnt->ValueForKey("targetname"), oldInput);
//		m_chunk->DumpConnections();
		exit(1);
	}
	m_connections[i].m_input = AllocStr(newInput);
}

//-----------------------------------------------------------------------------
const char *vmf_entity::UniqueName()
{
	// info_landmark + trigger_changelevel + trigger_transition share the same targetname
	const char *classname = ValueForKey("classname");
	if (!Q_stricmp(classname, "info_landmark") ||
		!Q_stricmp(classname, "trigger_changelevel") ||
		!Q_stricmp(classname, "trigger_transition"))
		return m_converter->m_keyvalues ? m_converter->m_keyvalues->ValueForKey("targetname") : ValueForKey("targetname");

	if (m_TargetName)
		return m_TargetName->UniqueNameForEntity(this);
	
	m_bReferredToByOthers = true;

	// If targetname is "", create a unique name as classname.id
	static char uniqueName[128];
	sprintf(uniqueName, "%s.%s", EntityConverter::ConvertedClassname( ValueForKey("classname") ), ValueForKey("id"));
	return uniqueName;
}

//-----------------------------------------------------------------------------
vmf_entity *vmf_entity::AddEntity(const char *classname, const char *targetname, const char *converterClassname)
{
	// new "entity" chunk
	const char *chunkName1 = "entity";
	const char *chunkName2 = AllocStr(chunkName1);
	vmf_chunk *chunk = new vmf_chunk(chunkName2);

	vmf_chunk::s_root.AddChild(chunk);

//	num_chunks++;

	// This is the *converted* classname
	chunk->AddKeyValue("classname", classname);

	if (targetname != NO_TARGET_NAME)
	{
		if (targetname == NULL)
		{
			char buf[128];
			sprintf(buf, "%s.%s.%s", UniqueName(), classname, ValueForKey("id"));
			targetname = buf;
		}
		chunk->AddKeyValue("targetname", targetname);
	}

	KeyValue *kv = m_chunk->m_keyvalues->Find("origin");
	if (kv == NULL && m_converter != NULL && m_converter->m_keyvalues != NULL)
		kv = m_converter->m_keyvalues->Find("origin");
	if (kv == NULL)
	{
		CEMsg("vmf_entity::AddEntity: bogus origin for %s\n", UniqueName());
		exit(1);
	}
	chunk->AddKeyValue("origin", kv->m_value);

	vmf_entity *entity = new vmf_entity(chunk);

	g_last_entity->m_next = entity;
	g_last_entity = entity;

//	g_num_entities++;

	if (targetname != NO_TARGET_NAME)
	{
		TargetName *tn = new TargetName(targetname);
		tn->AddEntity(entity);
		g_targetnames.Insert(targetname, tn);
	}

	entity->CreateConverter(converterClassname);
//	entity->m_converter->Init(entity);
	entity->m_converter->m_converted = true;

	return entity;
}

//-----------------------------------------------------------------------------
vmf_entity *vmf_entity::AddEnvMessage(const char *message)
{
#if 1
	vmf_entity *entity = AddEntity("env_message");
	entity->m_chunk->AddKeyValue("message", message);
	return entity;
#else
	// new "entity" chunk
	char *chunkName1 = "entity";
	char *chunkName2 = new char [strlen(chunkName1) + 1];
	strcpy(chunkName2, chunkName1);
	vmf_chunk *chunk = new vmf_chunk(chunkName2);

	vmf_chunk::s_root.AddChild(chunk);

//	num_chunks++;

	chunk->AddKeyValue("classname", "env_message");

	char targetname[128];
	sprintf(targetname, "%s.%s.%s", UniqueName(), "env_message", ValueForKey("id"));
	chunk->AddKeyValue("targetname", targetname);

	chunk->AddKeyValue("message", message);

	KeyValue *kv = m_chunk->m_keyvalues->Find("origin");
	if (kv == NULL && m_converter != NULL && m_converter->m_keyvalues != NULL)
		kv = m_converter->m_keyvalues->Find("origin");
	if (kv == NULL)
	{
		CEMsg("vmf_entity::AddEnvMessage: bogus origin for %s\n", UniqueName());
		exit(1);
	}
	chunk->AddKeyValue("origin", kv->m_value);

	vmf_entity *entity = new vmf_entity(chunk);

#if 1
	g_last_entity->m_next = entity;
	g_last_entity = entity;
#else
	// NOTE: add to head of entity list so we don't try to convert it
	entity->m_next = g_entities;
	g_entities = entity;
#endif

//	g_num_entities++;

	TargetName *tn = new TargetName(targetname);
	tn->AddEntity(entity);
	g_targetnames.Insert(targetname, tn);

	entity->m_converter = new EC_env_message;
	entity->m_converter->Init(entity);
	entity->m_converter->m_converted = true;

	return entity;
#endif
}

//-----------------------------------------------------------------------------
vmf_entity *vmf_entity::AddEnvTaskText(void)
{
	// Only one of these is needed on a map
	vmf_entity *entity = AddEntity("env_tasktext", "env_tasktext");
	return entity;
}

//-----------------------------------------------------------------------------
vmf_entity *vmf_entity::AddTriggerSoundscape(void)
{
#if 1
	vmf_entity *entity = AddEntity("trigger_soundscape");
	entity->m_chunk->AddKeyValue("soundscape", UniqueName());
	return entity;
#else
	// new "entity" chunk
	char *chunkName1 = "entity";
	char *chunkName2 = new char [strlen(chunkName1) + 1];
	strcpy(chunkName2, chunkName1);
	vmf_chunk *chunk = new vmf_chunk(chunkName2);

	vmf_chunk::s_root.AddChild(chunk);

//	num_chunks++;

	chunk->AddKeyValue("classname", "trigger_soundscape");

	char targetname[128];
	sprintf(targetname, "%s.%s", UniqueName(), "trigger_soundscape");
	chunk->AddKeyValue("targetname", targetname);

	chunk->AddKeyValue("soundscape", UniqueName());
	chunk->AddKeyValue("origin", ValueForKey("origin"));

	vmf_entity *entity = new vmf_entity(chunk);

	g_last_entity->m_next = entity;
	g_last_entity = entity;

//	g_num_entities++;

	TargetName *tn = new TargetName(targetname);
	tn->AddEntity(entity);
	g_targetnames.Insert(targetname, tn);

	entity->m_converter = new EntityConverter;
	entity->m_converter->Init(entity);
	entity->m_converter->m_converted = true;

	return entity;
#endif
}

//-----------------------------------------------------------------------------
vmf_entity *vmf_entity::CreateDummy(const char *classname, const char *targetname)
{
	TargetName *tn = LookupTargetName(targetname, 0);
	if (tn)
	{
		fprintf(stdout, "vmf_entity::CreateDummy: targetname '%s' exists\n", targetname);
//		exit(1);
		return 0;
	}

	// new "entity" chunk
	const char *chunkName1 = "entity";
	const char *chunkName2 = AllocStr(chunkName1);
	vmf_chunk *chunk = new vmf_chunk(chunkName2);

	vmf_chunk::s_root.AddChild(chunk);

//	num_chunks++;

	chunk->AddKeyValue("classname", classname);
	chunk->AddKeyValue("targetname", targetname);

	vmf_entity *entity = new vmf_entity(chunk);

	g_last_entity->m_next = entity;
	g_last_entity = entity;

//	g_num_entities++;

	tn = new TargetName(targetname);
	tn->AddEntity(entity);
	g_targetnames.Insert(targetname, tn);

	entity->CreateConverter();
	entity->m_converter->m_converted = true;
	entity->m_chunk->m_ignore = true;

	return entity;
}

//-----------------------------------------------------------------------------
void vmf_entity::ReplaceTextures(const char *textures[])
{
	for (vmf_chunk *solid = m_chunk->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		for (vmf_chunk *side = solid->LookupChild("side");
			side != 0;
			side = side->LookupNext("side"))
		{
			KeyValue *kv = side->m_keyvalues->Find("material");
			if (kv == NULL)
				continue;

			for (int i = 0; textures[i]; i += 2)
			{
				const char *oldTexture = textures[i];
				const char *newTexture = textures[i+1];

				if (!oldTexture[0] || (Q_stristr(kv->m_value, oldTexture) != NULL))
					kv->SetValue(newTexture);
			}
		}
	}
}

//-----------------------------------------------------------------------------
vmf_chunk::vmf_chunk(const char *name)
{
	m_name = name; // malloc'd
	m_keyvalues = new KeyValues;
	m_children = 0;
	m_next = 0;
	m_ignore = false;
}

//-----------------------------------------------------------------------------
void vmf_chunk::AddKeyValue(const char *key, const char *value)
{
	m_keyvalues->AddKeyValue(key, value);
}

//-----------------------------------------------------------------------------
void vmf_chunk::SetKeyValue(const char *key, const char *value)
{
	m_keyvalues->SetKeyValue(key, value);
}

//-----------------------------------------------------------------------------
void vmf_chunk::SetKeyValue(KeyValue *kv, const char *value)
{
	kv->SetValue(value);
}

//-----------------------------------------------------------------------------
bool vmf_chunk::HasKeyValue(const char *key, const char *value)
{
	return m_keyvalues->HasKeyValue(key, value);
}

//-----------------------------------------------------------------------------
bool vmf_chunk::HasKey(const char *key)
{
	return m_keyvalues->HasKey(key);
}

//-----------------------------------------------------------------------------
void vmf_chunk::AddChild(vmf_chunk *child)
{
	vmf_chunk *last = m_children;
	if (!last)
		m_children = child;
	else
	{
		while (last->m_next)
			last = last->m_next;
		last->m_next = child;
	}
}

//-----------------------------------------------------------------------------
vmf_chunk *vmf_chunk::LookupChild(char *name)
{
	vmf_chunk *child = m_children;
	while (child)
	{
		if (!Q_strcmp(child->m_name, name))
			return child;
		child = child->m_next;
	}
	return 0;
}

//-----------------------------------------------------------------------------
vmf_chunk *vmf_chunk::LookupNext(char *name)
{
	vmf_chunk *sibling = m_next;
	while (sibling)
	{
		if (!Q_strcmp(sibling->m_name, name))
			return sibling;
		sibling = sibling->m_next;
	}
	return 0;
}

//-----------------------------------------------------------------------------
void vmf_chunk::RemoveChild(vmf_chunk *remove)
{
	vmf_chunk *child = m_children;
	vmf_chunk *prev = 0;
	while (child)
	{
		if (child == remove)
		{
			if (prev)
				prev->m_next = remove->m_next;
			else
				m_children = remove->m_next;
			return;
		}
		prev = child;
		child = child->m_next;
	}
	fprintf(stdout, "vmf_chunk::RemoveChild: couldn't find child");
	exit(1);
	return;
}

//-----------------------------------------------------------------------------
void vmf_chunk::AddConnection(
	const char *output,
	const char *target,
	const char *input,
	const char *param,
	const char *delay,
	int onlyOnce)
{
	char value[512];
	sprintf(value, "%s,%s,%s,%s,%d", target, input, param, delay, onlyOnce);

	vmf_chunk *connections = LookupChild("connections");
	if (!connections)
	{
		connections = new vmf_chunk("connections");
		AddChild(connections);
	}
	// Don't add the same keyvalue more than once. This is because we have
	// multiple targets with the same name and don't want to Toggle/Toggle/Toggle
	// etc.
	if (!connections->HasKeyValue(output, value))
		connections->AddKeyValue(output, value);
}

//-----------------------------------------------------------------------------
void vmf_chunk::DumpConnections()
{
	fprintf(stdout, "Connections for \"%s\":\n", ValueForKey("targetname"));

	vmf_chunk *connections = LookupChild("connections");
	if (!connections)
		return;

	KeyValue *kv = connections->m_keyvalues->m_keyvalues;
	while (kv)
	{
		fprintf(stdout, "    %s %s\n", kv->m_key, kv->m_value);
		kv = kv->m_next;
	}
}

//-----------------------------------------------------------------------------
void vmf_chunk::Write(CChunkFile *pFile)
{
	// Skip removed entities
	if (m_ignore)
		return;

	if (this != &s_root)
		pFile->BeginChunk(m_name);

	// write key/value pairs
	KeyValue *kv = m_keyvalues->m_keyvalues;
	while (kv)
	{
		pFile->WriteKeyValue(kv->m_key, kv->m_value);
		kv = kv->m_next;
	}

	// write child chunks
	vmf_chunk *child = m_children;
	while (child != 0)
	{
		child->Write(pFile);
		child = child->m_next;
	}

	if (this != &s_root)
		pFile->EndChunk();
}

//-----------------------------------------------------------------------------
const char *vmf_chunk::ValueForKey(const char *key, const char *default)
{
	return m_keyvalues->ValueForKey(key, default);
}


//-----------------------------------------------------------------------------
TargetName::TargetName(const char *name)
{
	Q_strncpy(m_sTargetName, name, sizeof(m_sTargetName));
	m_bIsSharedByDifferentClasses = false;
}

//-----------------------------------------------------------------------------
void TargetName::AddEntity(vmf_entity *entity)
{
	entity->m_TargetName = this;

	m_EntityList.AddToTail(entity);
	
	if (m_bIsSharedByDifferentClasses)
		return;

	const char *className1 = entity->Classname();
	for (int i = 0; i < m_EntityList.Count(); i++)
	{
		const char *className2 = m_EntityList[i]->Classname();
		if (Q_strcmp(className1, className2))
		{
			m_bIsSharedByDifferentClasses = true;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
bool TargetName::IsSharedByDifferentClasses()
{
	return m_bIsSharedByDifferentClasses;
}

//-----------------------------------------------------------------------------
const char *TargetName::GetName() const
{
	return m_sTargetName;
}

//-----------------------------------------------------------------------------
const TargetName::EntityListType &TargetName::GetEntityList() const
{
	return m_EntityList;
}

//-----------------------------------------------------------------------------
const char *TargetName::UniqueNameForEntity(vmf_entity *entity)
{
	if (IsSharedByDifferentClasses())
	{
		Q_snprintf(s_sUniqueName, sizeof(s_sUniqueName), "%s.%s", GetName(),
			EntityConverter::ConvertedClassname(entity->Classname()));
		return s_sUniqueName;
	}
	return GetName();
}

ChunkFileResult_t LoadMapFile(const char *pszFileName)
{
	//
	// Open the file.
	//
	CChunkFile File;
	ChunkFileResult_t eResult = File.Open(pszFileName, ChunkFile_Read);

	//
	// Read the file.
	//
	if (eResult == ChunkFile_Ok)
	{
		//
		// Set up handlers for the subchunks that we are interested in.
		//
#if 1
		File.SetDefaultChunkHandler(LoadChunkCallback, &vmf_chunk::s_root);
#else
		CChunkHandlerMap Handlers;

		Handlers.AddHandler("world", (ChunkHandler_t)LoadEntityCallback, 0);
		Handlers.AddHandler("entity", (ChunkHandler_t)LoadEntityCallback, 0);
		File.PushHandlers(&Handlers);
#endif
		//
		// Read the sub-chunks. We ignore keys in the root of the file.
		//
		while (eResult == ChunkFile_Ok)
		{
			eResult = File.ReadChunk();
		}

//		File.PopHandlers();
	}
	else
	{
		fprintf(stdout, "Error opening %s: %s.\n", pszFileName, File.GetErrorText(eResult));
	}

	if ((eResult == ChunkFile_Ok) || (eResult == ChunkFile_EOF))
	{
	}
	else
	{
		fprintf(stdout, "Error processing %s: %s.\n", pszFileName, File.GetErrorText(eResult));
	}

	return eResult;
}

ChunkFileResult_t LoadKeyValueCallback(const char *szKey, const char *szValue, void *pData)
{
	// strip out empty key/value pairs created while importing an old map
	if (!szKey[0] && !szValue[0])
		return(ChunkFile_Ok);

	vmf_chunk *chunk = (vmf_chunk *) pData;

	chunk->AddKeyValue(szKey, szValue);

	return(ChunkFile_Ok);
}

ChunkFileResult_t LoadChunkCallback(CChunkFile *pFile, void *pData, char const *pChunkName)
{
	const char *name = AllocStr(pChunkName);
	vmf_chunk *chunk = new vmf_chunk(name);

	vmf_chunk *parent = (vmf_chunk *) pData;
	parent->AddChild(chunk);

	num_chunks++;
	
	if (!_stricmp(pChunkName, "entity"))
	{
		vmf_entity *entity = new vmf_entity(chunk);

		if (g_last_entity)
			g_last_entity->m_next = entity;
		else
			g_entities = entity;
		g_last_entity = entity;

		g_num_entities++;
	}

	pFile->SetDefaultChunkHandler(LoadChunkCallback, chunk);
	ChunkFileResult_t eResult = pFile->ReadChunk((KeyHandler_t)LoadKeyValueCallback, chunk);
	pFile->SetDefaultChunkHandler(LoadChunkCallback, parent);

	return eResult;
}

vmf_entity *FindEntity(vmf_entity *first, const char *name)
{
	vmf_entity *entity = first ? first->m_next : g_entities;
	while (entity)
	{
		if (!Q_stricmp(entity->ValueForKey("targetname"), name))
			return entity;
		entity = entity->m_next;
	}
	return 0;
}

vmf_entity *FindEntityByClass(vmf_entity *first, const char *name)
{
	vmf_entity *entity = first ? first->m_next : g_entities;
	while (entity)
	{
		if (!Q_stricmp(entity->ValueForKey("classname"), name))
			return entity;
		entity = entity->m_next;
	}
	return 0;
}

TargetName *LookupTargetName(const char *targetname, int flags)
{
	int iIndex = g_targetnames.Find(targetname);
	if (iIndex != g_targetnames.InvalidIndex())
		return g_targetnames.Element(iIndex);

	if (flags & 1)
		fprintf(stdout, "*** WARNING: LookupTargetName \"%s\" failed\n", targetname);
	if (flags & 2)
		exit(1);
	return 0;
}

TargetName *LookupTargetNameFailOK(const char *targetname)
{
	return LookupTargetName(targetname, LTN_MSG);
}

CallersFixup *CallersFixup::s_first = 0;

void CallersFixup::Init(vmf_entity *entity)
{
	m_entity = entity;
	m_next = 0;

	if (s_first)
	{
		CallersFixup *last = s_first;
		while (last->m_next)
			last = last->m_next;
		last->m_next = this;
	}
	else
		s_first = this;
}

void CallersFixup::FixupAll()
{
	CallersFixup *fixup = s_first;
	while (fixup)
	{
		fixup->Fixup();
		fixup = fixup->m_next;
	}
}

void CallersFixup_multisource::Fixup()
{
	int i;
	for (i = 0; i < m_entity->m_callers.Count(); i++)
	{
		char input[32];
		sprintf(input, "ToggleValue%02d", i + 1);
		m_entity->m_callers[i]->ChangeConnectionInput(m_entity, "FIXME", input);
		
		char value[32];
		sprintf(value, "Value%02d", i + 1);
		m_entity->m_chunk->AddKeyValue(value, "0"); // value is off
	}
	fprintf(stdout, "callers fixup (multisource): %d connections to \"%s\"\n", i,
		m_entity->ValueForKey("targetname"));
	for (; i < 8; i++)
	{
		char value[32];
		sprintf(value, "Value%02d", i + 1);
		m_entity->m_chunk->AddKeyValue(value, "1"); // value is on
	}
}

void CallersFixup_player_loadsaved::Fixup()
{
	for (int i = 0; i < m_entity->m_callers.Count(); i++)
	{
		vmf_entity *caller = m_entity->m_callers[i];
		caller->AddConnection(
			m_env_message,
			"", // param
			m_messagetime,
			ONLY_ONCE_FALSE); // only once

		fprintf(stdout, "callers fixup (player_loadsaved): %s -> %s\n", caller->UniqueName(),
			m_entity->UniqueName());
	}
}

#if 0
void FixMultiSourceConnections( )
{
	vmf_entity *entity = g_entities;
	while (entity != 0)
	{
		if (!Q_strcmp(entity->Classname(), "logic_and"))
		{
			int i;
			for (i = 0; i < entity->m_callers.Count(); i++)
			{
				char input[32];
				sprintf(input, "ToggleValue%02d", i + 1);
				entity->m_callers[i]->ChangeConnectionInput(entity, "FIXME", input);
				
				char value[32];
				sprintf(value, "Value%02d", i + 1);
				entity->m_chunk->AddKeyValue(value, "0"); // value is off
			}
			fprintf(stdout, "FixMultiSourceConnections: %d connections to \"%s\"\n", i,
				entity->ValueForKey("targetname"));
			for (; i < 8; i++)
			{
				char value[32];
				sprintf(value, "Value%02d", i + 1);
				entity->m_chunk->AddKeyValue(value, "1"); // value is on
			}
		}
		entity = entity->m_next;
	}
}
#endif

//-----------------------------------------------------------------------------
void FixWaterTextures(vmf_chunk *solid, const char *waterTextures[])
{
	for (vmf_chunk *side = solid->LookupChild("side");
		side != 0;
		side = side->LookupNext("side"))
	{
		KeyValue *kv = side->m_keyvalues->Find("material");
		if (kv == NULL)
			continue;

		for (int i = 0; waterTextures[i]; i++)
		{
			if (Q_stristr(kv->m_value, waterTextures[i]) != NULL)
			{
				const char *plane = side->m_keyvalues->ValueForKey("plane");
				vector v1, v2, v3;
				if ((sscanf(plane, "(%f %f %f) (%f %f %f) (%f %f %f)",
					&v1.x, &v1.y, &v1.z,
					&v2.x, &v2.y, &v2.z,
					&v3.x, &v3.y, &v3.z) == 9) &&
					(v1.z != v2.z || v2.z != v3.z))
				{
					// FIXME: this doesn't set the bottom face
					kv->SetValue("tools/toolsnodraw");
				}
				break;
			}
		}
	}
}

void FixWaterTextures(const char *waterTextures[])
{
	vmf_chunk *world = vmf_chunk::s_root.LookupChild("world");
	if (world == NULL)
	{
		CEMsg("can't find the world\n");
		exit(1);
	}

	for (vmf_chunk *solid = world->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		FixWaterTextures(solid, waterTextures);
	}

	vmf_entity *entity = g_entities;
	while (entity != 0)
	{
		for (vmf_chunk *solid = entity->m_chunk->LookupChild("solid");
			solid != 0;
			solid = solid->LookupNext("solid"))
		{
			FixWaterTextures(solid, waterTextures);
		}
		entity = entity->m_next;
	}
}

#define LADDERS_ONLY 1
#ifdef LADDERS_ONLY
static int FixLaddersOnly( const char *path )
{
	ChunkFileResult_t eResult = LoadMapFile(path);
	if ((eResult == ChunkFile_Ok) || (eResult == ChunkFile_EOF))
	{
		fprintf(stdout, "loaded %d chunks, %d entities\n", num_chunks, g_num_entities);
		fflush(stdout);

		vmf_entity *entity = g_entities;
		while (entity != 0)
		{
			if (!Q_stricmp(entity->Classname(), "func_ladder"))
			{
				entity->CreateConverter();
				entity->Convert();
			}
			entity = entity->m_next;
		}

		// Write
		CChunkFile File;
		fprintf(stdout, "writing %s...\n", path);
		eResult = File.Open(path, ChunkFile_Write);
		vmf_chunk::s_root.Write(&File);
	}

	return 0;
}
#endif // LADDERS_ONLY

#define FIX_INFODECALS 1
#ifdef FIX_INFODECALS
// Add "LowPriority 0" to every infodecal that doesn't specify it
static int FixInfoDecals( const char *fIn, const char *fOut )
{
	ChunkFileResult_t eResult = LoadMapFile(fIn);
	if ((eResult == ChunkFile_Ok) || (eResult == ChunkFile_EOF))
	{
		fprintf(stdout, "loaded %d chunks, %d entities\n", num_chunks, g_num_entities);
		fflush(stdout);

		int count = 0;
		vmf_entity *entity = g_entities;
		while (entity != 0)
		{
			if (entity->IsClassname("infodecal"))
			{
				if (!entity->HasKey("LowPriority"))
				{
					entity->m_chunk->AddKeyValue("LowPriority", "0");
					count++;
				}
			}
			entity = entity->m_next;
		}
		CEMsg("fixed %d infodecal entities\n", count);

		// Write
		CChunkFile File;
		fprintf(stdout, "writing %s...\n", fOut);
		eResult = File.Open(fOut, ChunkFile_Write);
		vmf_chunk::s_root.Write(&File);
	}

	return 0;
}
#endif // FIX_INFODECALS

int _tmain(int argc, _TCHAR* argv[])
{
	if ((argc < 3) || (argc > 4) || (argc == 4 && argv[1][0] != '-'))
	{
		printf("usage: %s [-option] input.vmf output.vmf", argv[0]);
		exit(0);
	}

	if ( argc == 4 )
	{
#ifdef LADDERS_ONLY
		if ( !Q_stricmp(argv[1], "-ladders") )
			return FixLaddersOnly( argv[2] );
#endif
#ifdef FIX_INFODECALS
		if ( !Q_stricmp(argv[1], "-infodecals") )
			return FixInfoDecals( argv[2], argv[3] );
#endif
		CEMsg("bogus option '%s'\n", argv[1]);
		exit(1);
	}

	ChunkFileResult_t eResult = LoadMapFile(argv[1]);

	if ((eResult == ChunkFile_Ok) || (eResult == ChunkFile_EOF))
	{
		fprintf(stdout, "loaded %d chunks, %d entities\n", num_chunks, g_num_entities);
		fflush(stdout);

		vmf_entity *entity = g_entities;
		while (entity != 0)
		{
			const char *targetname = entity->ValueForKey("targetname");
#if 0
			// If this entity has a master but no targetname, then give it a
			// targetname so the master can call one of its inputs.
			if (!targetname[0] && entity->ValueForKey("master")[0])
			{
				char nameIfWeHaveAMaster[128];
				sprintf(nameIfWeHaveAMaster, "%s.%s",
					EntityConverter::ConvertedClassname(entity->Classname()),
					entity->ValueForKey("id"));
				KeyValue *kv = entity->m_chunk->m_keyvalues->Find("targetname");
				if (kv)
					kv->SetValue(nameIfWeHaveAMaster);
				else
					entity->m_chunk->AddKeyValue("targetname", nameIfWeHaveAMaster);
				targetname = entity->ValueForKey("targetname");
			}
#endif
			// Make a list of all entities that share this name
			if (targetname[0])
			{
				int iIndex = g_targetnames.Find(targetname);
				if (iIndex == g_targetnames.InvalidIndex())
				{
					TargetName *tn = new TargetName(targetname);
					tn->AddEntity(entity);
					g_targetnames.Insert(targetname, tn);
				}
				else
				{
					TargetName *tn = g_targetnames.Element(iIndex);
					tn->AddEntity(entity);
				}
			}
			entity = entity->m_next;
		}

		for (int i = g_targetnames.First();
			i != g_targetnames.InvalidIndex();
			i = g_targetnames.Next(i))
		{
			TargetName *tn = g_targetnames.Element(i);
			if (tn->IsSharedByDifferentClasses())
			{
				fprintf(stdout, "targetname \"%s\" is shared by different classes\n",
					tn->GetName());
			}
		}

		entity = g_entities;
		while (entity != 0)
		{
			entity->CreateConverter();
			entity = entity->m_next;
		}

		entity = g_entities;
		while (entity != 0)
		{
			entity->Convert();
			entity = entity->m_next;
		}

		entity = g_entities;
		while (entity != 0)
		{
			if (entity->m_converter)
				entity->m_converter->Finalize();
			entity = entity->m_next;
		}

		CallersFixup::FixupAll();

		// Ensure every target with callers has a targetname
		entity = g_entities;
		while (entity != 0)
		{
			if (entity->m_bReferredToByOthers && !entity->ValueForKey("targetname")[0])
			{
				entity->m_chunk->SetKeyValue("targetname", entity->UniqueName());
				fprintf(stdout, "gave %s a unique targetname\n", entity->ValueForKey("targetname"));
			}
			entity = entity->m_next;
		}

		// Replace HL1 textures with Source equivalents
		const char *replaceTextures[] = {
			"{invisible", "tools/toolsinvisible",
			NULL
		};
		entity = g_entities;
		while (entity != 0)
		{
			entity->ReplaceTextures(replaceTextures);
			entity = entity->m_next;
		}

		const char *waterTextures[] = {
			"!gwater3",
			"!gwater4a",
			"!gwater4b",
			"!gwater5",
			"!gwater5b",
			"!gwater6",
			"!mudwater",
			"!watergreen",
			"water4b",
			NULL
		};
		FixWaterTextures(waterTextures);

		// Fill in the "connections" chunk for each entity.
		entity = g_entities;
		while (entity != 0)
		{
			vmf_chunk &chunk = *entity->m_chunk;
			for (int i = 0; i < entity->m_connections.Count(); i++)
			{
				Connection &c = entity->m_connections[i];
				// If the entity to send an input to was removed, then skip the input
				if (c.m_entity->m_chunk->m_ignore)
				{
					// Okay to call UniqueName() here because m_bReferredToByOthers was handled above
					CEMsg("connection %s -> %s %s skipped because target was removed\n", entity->UniqueName(), c.m_entity->UniqueName(), c.m_input);
					continue;
				}
				chunk.AddConnection(c.m_output, c.m_entity->UniqueName(), c.m_input, c.m_param, c.m_delay, c.m_onlyOnce);
			}
			entity = entity->m_next;
		}


		// Write
		CChunkFile File;
		fprintf(stdout, "writing %s...\n", argv[2]);
		eResult = File.Open(argv[2], ChunkFile_Write);
		vmf_chunk::s_root.Write(&File);
	}
	return 0;
}
