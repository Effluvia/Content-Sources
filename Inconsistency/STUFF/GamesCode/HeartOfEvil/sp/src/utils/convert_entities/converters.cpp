#include "convert_entities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CUtlDict< const char *, int > EntityConverter::s_ConvertedClassName;
CUtlDict< EntityConverter::EntityFactoryFn, int > EntityConverter::s_factory;

//-----------------------------------------------------------------------------
BEGIN_CONVERTER_NO_BASE ( EntityConverter )
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(id),
	KEEP_KEYVALUE(origin),
	REMOVE_KEYVALUE(delay), // this is usally the delay before firing an output
	CLASSNAME_VALUE(classname),
	XFORM_KEYVALUE(killtarget, Handle_killtarget),
	XFORM_KEYVALUE(master, Handle_master),
	XFORM_KEYVALUE(rendercolor, Handle_rendercolor),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(target, Handle_target),
	XFORM_KEYVALUE(targetname, Handle_targetname),
END_CONVERTER()

void EntityConverter::Init(vmf_entity *entity)
{
	m_entity = entity;
	m_keyvalues = 0;
	m_converted = false;
}

void EntityConverter::Convert(void)
{
	// Hack so dynamically-created entities are not converted
	if (m_converted)
		return;
	m_converted = true;

	m_keyvalues = m_entity->m_chunk->m_keyvalues;
	m_entity->m_chunk->m_keyvalues = new KeyValues;

	// Handle SET_KEYVALUE fields
	// If I want to override "delay", I need to do it before processing the "target"
	// keyvalue.
	for (converter_datamap_t *map = GetDataDescMap(); map; map = map->baseMap)
	{
		for (int i = 0; i < map->dataNumFields; i++)
		{
			converter_typedescription_t &td = map->dataDesc[i];
			if (td.fieldType != FIELD_SET)
				continue;
			m_keyvalues->SetKeyValue(td.key, td.rename);
		}
	}

	/* Loop over old keyvalues. For each old keyvalue:
	 *   see if the keyvalue is in our map array
	 *   if in map array, handle it
	 *   if not in map array, check base class map array
	 *   if not in any map array, copy it
	 */

	for (KeyValue *kv = m_keyvalues->m_keyvalues; kv; kv = kv->m_next)
	{
		bool handled = false;
		
		for (converter_datamap_t *map = GetDataDescMap(); map; map = map->baseMap)
		{
			for (int i = 0; i < map->dataNumFields; i++)
			{
				converter_typedescription_t &td = map->dataDesc[i];
				if (td.fieldType == FIELD_VOID)
					continue;
				if (!Q_stricmp(td.key, kv->m_key))
				{
					switch (td.fieldType)
					{
					case FIELD_FUNC:
						(this->*td.func)(*kv);
						break;
					case FIELD_KEEP:
						AddKeyValue(*kv);
						break;
					case FIELD_REMOVE:
						/* nothing */
						break;
					case FIELD_RENAME:
						{
							// func_door in Goldsrc has a key called "angles" which is renamed "movedir" in Source.
							// But hammer will already have added a default "movedir" which will overwrite this value.
							KeyValue *nkv = m_entity->m_chunk->m_keyvalues->Find(td.rename);
							if (nkv)
								nkv->SetValue(kv->m_value);
							else
								AddKeyValue(td.rename, kv->m_value);
						}
						break;
					case FIELD_CLASSNAME:
						AddKeyValue(td.rename, ConvertedClassname(kv->m_value));
						break;
					case FIELD_ENTITY:
						{
							if (kv->m_value[0])
							{
								// First look for an entity with that name. If not found, assume
								// it is a classname and try to convert it. This is all to fix
								// "monster_barney" as a target in some scripted_sequences.
								vmf_entity *entity = FindEntity(NULL, kv->m_value);
								if (entity)
									AddKeyValue(td.rename, kv->m_value);
								else
								{
									AddKeyValue(td.rename, ConvertedClassname(kv->m_value));
								}
							}
							else
								AddKeyValue(*kv);
						}
						break;
					}
					handled = true;
					break;
				}
			}
			if (handled)
				break;
		}

		if (handled)
			continue;

		Handle_unknown(*kv); // keep it
	}

	// Handle ADD_KEYVALUE fields
	for (converter_datamap_t *map = GetDataDescMap(); map; map = map->baseMap)
	{
		for (int i = 0; i < map->dataNumFields; i++)
		{
			converter_typedescription_t &td = map->dataDesc[i];
			if (td.fieldType != FIELD_ADD)
				continue;

			// If keyvalue exists, don't add it again (see ambient_mp3)
			if (m_entity->m_chunk->m_keyvalues->HasKey(td.key))
				continue;

			AddKeyValue(td.key, td.rename);
		}
	}

	// Convert a brush with "origin" texture to "origin" keyvalue
	OriginBrushToKeyvalue(m_entity);

	// now free m_keyvalues
}

void EntityConverter::Finalize(void)
{
}

void EntityConverter::AddKeyValue(const char *key, const char *value)
{
	m_entity->m_chunk->AddKeyValue(key, value);
}

void EntityConverter::AddKeyValue(KeyValue &kv)
{
	m_entity->m_chunk->AddKeyValue(kv.m_key, kv.m_value);
}

void EntityConverter::SetKeyValue(const char *key, const char *value)
{
	m_entity->m_chunk->SetKeyValue(key, value);
}

void EntityConverter::SetSpawnflags(int sf)
{
	char buf[32];
	sprintf(buf, "%d", sf);
	SetKeyValue("spawnflags", buf);
}

void EntityConverter::UnsetSpawnflags(int sf)
{
	int newsf = atol(m_entity->ValueForKey("spawnflags")) & ~sf;
	char buf[32];
	sprintf(buf, "%d", newsf);
	SetKeyValue("spawnflags", buf);
}

//-----------------------------------------------------------------------------
const char *EntityConverter::ConvertedClassname(const char *classname)
{
	int i = s_ConvertedClassName.Find(classname);
	if (i != s_ConvertedClassName.InvalidIndex())
		return s_ConvertedClassName[i];
	return classname;
}

//-----------------------------------------------------------------------------
const char *EntityConverter::ConvertedClassname()
{
	if (GetDataDescMap()->convertedClassname != 0)
		return GetDataDescMap()->convertedClassname;
	return GetDataDescMap()->dataClassName;
}

//-----------------------------------------------------------------------------
EntityConverter *EntityConverter::CreateConverterByName(const char *classname)
{
	int i = s_factory.Find(classname);
	if (i != s_factory.InvalidIndex())
		return s_factory[i]();
	return new EntityConverter;
}

//-----------------------------------------------------------------------------
const char *EntityConverter::ToggleInput(vmf_entity *entity)
{
	if (GetDataDescMap()->toggleInput != 0)
		return GetDataDescMap()->toggleInput;
	if (GetDataDescMap()->baseMap && GetDataDescMap()->baseMap->toggleInput != 0)
		return GetDataDescMap()->baseMap->toggleInput;

	const char *classname = m_keyvalues ? m_keyvalues->ValueForKey("classname") : m_entity->Classname(); //ConvertedClassname();
	fprintf(stdout, "*** WARNING: can't determine best input for class \"%s\"\n", classname);
	return "???";
}

//-----------------------------------------------------------------------------
const char *EntityConverter::BestOutput(vmf_entity *entity)
{
	if (GetDataDescMap()->bestOutput != 0)
		return GetDataDescMap()->bestOutput;
	if (GetDataDescMap()->baseMap && GetDataDescMap()->baseMap->bestOutput != 0)
		return GetDataDescMap()->baseMap->bestOutput;

	const char *classname = m_keyvalues ? m_keyvalues->ValueForKey("classname") : m_entity->Classname(); //ConvertedClassname();
	fprintf(stdout, "*** WARNING: can't determine best output for class \"%s\"\n", classname);
	return "???";
}

//-----------------------------------------------------------------------------
const char *EntityConverter::MasterInput(vmf_entity *entity)
{
	if (GetDataDescMap()->masterInput != 0)
		return GetDataDescMap()->masterInput;
	if (GetDataDescMap()->baseMap && GetDataDescMap()->baseMap->masterInput != 0)
		return GetDataDescMap()->baseMap->masterInput;

	const char *classname = m_keyvalues ? m_keyvalues->ValueForKey("classname") : m_entity->Classname(); //ConvertedClassname();
	fprintf(stdout, "*** WARNING: can't determine best *master* input for class \"%s\"\n", classname);
	return "???";
}

//-----------------------------------------------------------------------------
const char *EntityConverter::OnInput(vmf_entity *entity)
{
	if (GetDataDescMap()->onInput != 0)
		return GetDataDescMap()->onInput;
	if (GetDataDescMap()->baseMap && GetDataDescMap()->baseMap->onInput != 0)
		return GetDataDescMap()->baseMap->onInput;

	const char *classname = m_keyvalues ? m_keyvalues->ValueForKey("classname") : m_entity->Classname(); //ConvertedClassname();
	fprintf(stdout, "*** WARNING: can't determine best trigger_relay ON input for class \"%s\"\n", classname);
	return "???";
}

//-----------------------------------------------------------------------------
const char *EntityConverter::OffInput(vmf_entity *entity)
{
	if (GetDataDescMap()->offInput != 0)
		return GetDataDescMap()->offInput;
	if (GetDataDescMap()->baseMap && GetDataDescMap()->baseMap->offInput != 0)
		return GetDataDescMap()->baseMap->offInput;

	const char *classname = m_keyvalues ? m_keyvalues->ValueForKey("classname") : m_entity->Classname(); //ConvertedClassname();
	fprintf(stdout, "*** WARNING: can't determine best trigger_relay OFF input for class \"%s\"\n", classname);
	return "???";
}

//-----------------------------------------------------------------------------
void EntityConverter::Handle_unknown(KeyValue &kv)
{
	// See comment above near FIELD_RENAME
	if (m_entity->m_chunk->m_keyvalues->Find(kv.m_key))
	{
		fprintf(stdout, "*** ignoring duplicate key \"%s\" for entity \"%s\" \n", kv.m_key, m_entity->UniqueName());
		return;
	}
	AddKeyValue(kv);
}

void EntityConverter::Handle_killtarget(KeyValue &kv)
{
	// Send "Kill" input to any "killtarget" entities
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (!tn) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				BestOutput(entityList[i]),
				entityList[i],
				"Kill", // FIXME: KillHeirarchy?
				"",
				m_keyvalues->ValueForKey("delay", "0"), // FIXME: or always "0"?
				ONLY_ONCE_FALSE);
		}
	}
}

void EntityConverter::Handle_master(KeyValue &kv)
{
#if 1
	AddKeyValue(kv);
#else
	// Tell our master(s) to call our Unlock/Enabled/whatever input
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (!tn) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			entityList[i]->AddConnection(
				entityList[i]->m_converter->BestOutput(m_entity),
				m_entity,
				MasterInput(entityList[i]),
				"",
				"0", // FIXME: get delay from master?
				ONLY_ONCE_FALSE);

			fprintf(stdout, "ADDED connection from master \"%s\" -> \"%s\"\n",
				entityList[i]->UniqueName(), m_entity->UniqueName());
		}
	}
#endif
}

void EntityConverter::Handle_rendercolor(KeyValue &kv)
{
	const char *rendercolor = kv.m_value;
	const char *renderfx = m_keyvalues->ValueForKey("renderfx");
	const char *rendermode = m_keyvalues->ValueForKey("rendermode");

	// This next bit is from MapFOOL, the "RenderColor bug"
	// Dont' mess with FADE however.
	if (!m_entity->IsClassname("env_fade") &&
		!m_entity->IsClassname("player_loadsaved") &&
		/*(!renderfx[0] || !Q_strcmp(renderfx, "0")) &&
		(!rendermode[0] || !Q_strcmp(rendermode, "0")) &&*/
		!Q_strcmp(rendercolor, "0 0 0"))
	{
		AddKeyValue("rendercolor", "255 255 255");
//		fprintf(stdout, "converted %s #%s rendercolor 0,0,0 -> 255,255,255\n",
//			m_keyvalues->ValueForKey("classname"), m_keyvalues->ValueForKey("id"));
	}
	else
		AddKeyValue("rendercolor", rendercolor);
}

void EntityConverter::Handle_spawnflags(KeyValue &kv)
{
	// Remove all instances of spawnflags==0
	if (Q_strcmp(kv.m_value, "0"))
		AddKeyValue(kv);
}

void EntityConverter::Handle_target(KeyValue &kv)
{
	// Add one connection for each target matching our "target" entity name.
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				entityList[i],
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

void EntityConverter::Handle_targetname(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetName(kv.m_value);
		AddKeyValue(kv.m_key, m_entity->UniqueName());
		return;
	}
	AddKeyValue(kv.m_key, kv.m_value);
}

bool EntityConverter::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	return false;
}

KeyValues &EntityConverter::OriginalKeyValues(void)
{
	return m_keyvalues ? *m_keyvalues : *m_entity->m_chunk->m_keyvalues;
}

KeyValues &EntityConverter::OriginalKeyValues(vmf_entity *entity)
{
	if (entity->m_converter != NULL && entity->m_converter->m_keyvalues != NULL)
		return *entity->m_converter->m_keyvalues;
	return *entity->m_chunk->m_keyvalues;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_ambient_generic )
	OFF_INPUT(StopSound)
	ON_INPUT(PlaySound)
	TOGGLE_INPUT(ToggleSound)
	BEGIN_KEYVALUES()
	RENAME_KEY(fadein, fadeinsecs),
	RENAME_KEY(fadeout, fadeoutsecs),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_ambient_generic::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 16 | 32);
	if (sf & 1) // everywhere
		AddKeyValue("radius", "-1"); // not really needed
	else if (sf & 2) // small radius
		AddKeyValue("radius", "800");
	else if (sf & 4) // medium radius
		AddKeyValue("radius", "1250");
	else if (sf & 8) // large radius
		AddKeyValue("radius", "2000");
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_ambient_mp3 )
	CONVERTED_CLASSNAME(ambient_generic)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(duration),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	ADD_KEYVALUE(spawnflags, 1), // play everywhere (default if spawnflags keyvalue isn't found)
END_CONVERTER()

void EC_ambient_mp3::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = 1; // play everywhere
	if (!(sf & 1)) // not looped
		newsf |= 32;
	if (!(sf & 2)) // don't start on
		newsf |= 16;
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_beam )
	OFF_INPUT(TurnOff)
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_beverage )
	ON_INPUT(Activate) // FIXME: not sure about this
	TOGGLE_INPUT(Activate) // FIXME: not sure about this
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(angles)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_bush )
	CONVERTED_CLASSNAME(prop_bush)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_explosion )
	ON_INPUT(Explode)
	TOGGLE_INPUT(Explode)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_fade )
	ON_INPUT(Fade)
	TOGGLE_INPUT(Fade)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_fog )
	CONVERTED_CLASSNAME(env_fog_controller)
	ON_INPUT(TurnOn)
	OFF_INPUT(TurnOff)
	TOGGLE_INPUT(TurnOn)
	BEGIN_KEYVALUES()
	RENAME_KEY(startdist, fogstart),
	RENAME_KEY(enddist, fogend),
	RENAME_KEY(rendercolor, fogcolor),
END_CONVERTER()


//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_global )
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(triggermode),
END_CONVERTER()

const char *EC_env_global::OffInput( vmf_entity *entity )
{
	return ToggleInput( entity );
}

const char *EC_env_global::OnInput( vmf_entity *entity )
{
	return ToggleInput( entity );
}

const char *EC_env_global::ToggleInput( vmf_entity *entity )
{
	KeyValues &keyvalues = OriginalKeyValues();
	const char *triggermode = keyvalues.ValueForKey("triggermode", "0");
	int ts = Q_atoi(triggermode);
	switch (ts)
	{
	case 0:
		return "TurnOff";
	case 1:
		return "TurnOn";
	case 2:
		return "Remove";
	case 3:
		return "Toggle";
	}

	CEMsg("*** WARNING bogus triggermode value %d for env_global\n", ts);
	return "Toggle";
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_message )
	ON_INPUT(ShowMessage)
	TOGGLE_INPUT(ShowMessage)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(message, Handle_message)
END_CONVERTER()

void EC_env_message::Convert(void)
{
	// If displaying a letter then delete this env_message; the calling func_button will be converted to func_letter
	const char *message = m_entity->ValueForKey("message");
	if (LetterName(message) != NULL)
	{
		m_entity->m_chunk->m_ignore = true;
		return;
	}

	BaseClass::Convert();
}

bool EC_env_message::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{		
	// If this env_message is called by a button then display using env_tasktext instead.
	if (caller->IsClassname("func_button") || caller->IsClassname("func_rot_button"))
	{
		vmf_entity *env_tasktext = FindEntityByClass(NULL, "env_tasktext");
		if (env_tasktext == NULL)
			env_tasktext = m_entity->AddEnvTaskText();

		char buf[128];
		const char *message = m_entity->ValueForKey("message");
		if (message[0] != '#')
			Q_snprintf(buf, 128, "#%s", message);
		else
			Q_strcpy(buf, message);

		caller->AddConnection(output, env_tasktext, "ShowTaskText", buf, "0", ONLY_ONCE_FALSE);

		// Delete this env_message
		m_entity->m_chunk->m_ignore = true;

		CEMsg("replaced env_message %s with env_tasktext\n", m_entity->ValueForKey("message"));

		return true;
	}
	return false;
}

void EC_env_message::Handle_message(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		if (kv.m_value[0] != '#')
		{
			char buf[512];
			Q_snprintf(buf, 512, "#%s", kv.m_value);
			AddKeyValue(kv.m_key, buf);
		}
		else
		{
			AddKeyValue(kv);
		}
	}
}

const char *EC_env_message::LetterName(const char *message)
{
	const char *titleToLetter[] = {
		"FREEMAN25", "freeman25",
		"FREEMAN27", "freeman27",
		"FREEMAN28", "freeman28",
		"FREEMAN29", "freeman29",
		"FREEMAN30", "freeman30",
		"FREEMAN31", "freeman31",
		"FREEMAN32", "freeman32",
		"FREEMAN33", "freeman33",
		"USE_DOCUMENT1", "document1",
		"USE_DOCUMENT2", "document2",
		"USE_DOCUMENT3", "document3",
		"USE_DOCUMENT4", "document4",
		"USE_DOCUMENT5", "document5",
		"USE_DOCUMENT6", "document6",
		"USE_DOCUMENT7", "document7",
		"USE_LETTER1", "letter1_4",
		"USE_LETTER2", "letter1_4",
		"USE_LETTER3", "letter1_4",
		"USE_LETTER4", "letter1_4",
		"USE_LETTER5", "letter5_6",
		"USE_LETTER6", "letter5_6",
		"USE_NOTICE1", "notice1",
		"USE_NOTICE2", "notice2",
		"USE_NOTICE3", "notice3",
		"USE_NOTICE4", "notice4",
		"USE_TECHNOTE1", "technote1",
		NULL
	};

	for (int i = 0; titleToLetter[i]; i += 2)
	{
		// Handle "foo" and "#foo"
		if (!Q_strcmp(message, titleToLetter[i]) || !Q_strcmp(message+1, titleToLetter[i]))
			return titleToLetter[i+1];
	}
	return NULL;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_palm ) // becomes a prop_dynamic so it can be shot apart
	CONVERTED_CLASSNAME(prop_palm)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(solid, 2) // Use hitbox for collision
//	ADD_KEYVALUE(model, models/palm/palm.mdl)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_render )
	ON_INPUT(FIXME)
	OFF_INPUT(FIXME)
	TOGGLE_INPUT(FIXME)
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_env_render::Convert( void )
{
	m_keyvalues = m_entity->m_chunk->m_keyvalues;
	m_entity->m_chunk->m_keyvalues = new KeyValues;
	m_entity->m_chunk->m_ignore = true;
}

bool EC_env_render::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// This can get called before Convert()
	KeyValues &keyvalues = OriginalKeyValues();

	// There is no env_render in Source, so just pass all inputs to this entity
	// to this entity's target(s)

	const char *target = keyvalues.ValueForKey("target");
	if (!target[0])
		return true;

	const char *renderamt = keyvalues.ValueForKey("renderamt");
	const char *rendercolor = keyvalues.ValueForKey("rendercolor", "0 0 0");
	const char *rendermode = keyvalues.ValueForKey("rendermode", "0");
	const char *renderfx = keyvalues.ValueForKey("renderfx");

	const char *spawnflags = keyvalues.ValueForKey("spawnflags");
	int sf = Q_atoi(spawnflags);

	bool bRenderModeNormal = false;

	if (!(sf & 1))
		fprintf(stdout, "*** WARNING: env_render renderfx \"%s\" ignored\n", renderfx);
	if (!(sf & 4))
	{
		// If setting rendermode to "normal", then set renderamt to fully opaque
		if (!Q_strcmp(rendermode, "0"))
		{
			renderamt = "255";
			bRenderModeNormal = true;
		}

		fprintf(stdout, "*** WARNING: env_render rendermode \"%s\" ignored\n", rendermode);
	}

	if (!Q_strcmp(rendercolor, "0 0 0"))
		rendercolor = "255 255 255"; // rendercolor bug

	TargetName *tn = LookupTargetName(target);
	const TargetName::EntityListType &entityList = tn->GetEntityList();
	for (int i = 0; i < entityList.Count(); i++)
	{
		if (!(sf & 2))
		{
			caller->AddConnection(output, entityList[i], "Alpha", renderamt, delay, onlyOnce);
		}
		if (!(sf & 8) && !bRenderModeNormal)
		{
			caller->AddConnection(output, entityList[i], "Color", rendercolor, delay, onlyOnce);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_shake )
	TOGGLE_INPUT(StartShake)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_shooter )
	ON_INPUT(Shoot)
	TOGGLE_INPUT(Shoot)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(delay), // delay between gibs shooting, not firing outputs
	REMOVE_KEYVALUE(scale),
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_sound )
	CONVERTED_CLASSNAME(env_soundscape_triggerable)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(roomtype, Handle_roomtype),
END_CONVERTER()

void EC_env_sound::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	// Add a trigger_soundscape that calls us
	// FIXME: add a unique "id" keyvalue
	(void) m_entity->AddTriggerSoundscape();
}

void EC_env_sound::Handle_roomtype(KeyValue &kv)
{
	// These are the names defined in scripts/soundscapes_hl1.txt which I got from HL:S.
	static const char *soundscapes[] = {
		"Normal (off)",
		"Generic",
		"Metal Small",
		"Metal Medium",
		"Metal Large",
		"Tunnel Small",
		"Tunnel Medium",
		"Tunnel Large",
		"Chamber Small",
		"Chamber Medium",
		"Chamber Large",
		"Bright Small",
		"Bright Medium",
		"Bright Large",
		"Water 1",
		"Water 2",
		"Water 3",
		"Concrete Small",
		"Concrete Medium",
		"Concrete Large",
		"Big 1",
		"Big 2",
		"Big 3",
		"Cavern Small",
		"Cavern Medium",
		"Cavern Large",
		"Weirdo 1",
		"Weirdo 2",
		"Weirdo 3",
	};

	int i = Q_atoi(kv.m_value);
	if (i < 0 || i > 28)
	{
		fprintf(stdout, "*** WARNING: env_sound has illegal roomtype %d, setting to zero\n", i);
		i = 0;
	}
	AddKeyValue("soundscape", soundscapes[i]);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_spark )
	TOGGLE_INPUT(ToggleSpark)
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_env_spark::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (64);
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}
//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_env_sprite )
	OFF_INPUT(HideSprite)
	ON_INPUT(ShowSprite)
	TOGGLE_INPUT(ToggleSprite)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(angles),
	XFORM_KEYVALUE(rendercolor, Handle_rendercolor),
END_CONVERTER()

// FIXME: base class also handles this, but differently
void EC_env_sprite::Handle_rendercolor(KeyValue &kv)
{
	const char *rendercolor = kv.m_value;

	if (!Q_strcmp(rendercolor, "0 0 0"))
	{
		AddKeyValue("rendercolor", "255 255 255");
	}
	else
		AddKeyValue("rendercolor", rendercolor);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_breakable ) // FIXME: func_breakable.delay -> pressuredelay?
	ON_INPUT(Break)
	OFF_INPUT(Break)
	TOGGLE_INPUT(Break)
	BEST_OUTPUT(OnBreak)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(explodemagnitude, Handle_explodemagnitude),
END_CONVERTER()

void EC_func_breakable::Handle_explodemagnitude(KeyValue &kv)
{
	int magnitude = Q_atoi(kv.m_value);
	if (magnitude > 0)
	{
		AddKeyValue("ExplodeRadius", kv.m_value);
		char szDmg[16];
		sprintf(szDmg, "%.2f", (float) magnitude * 2.5); // from HL1 RadiusDamage()
		AddKeyValue("ExplodeDamage", szDmg);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_button )
	TOGGLE_INPUT(Press)
	BEST_OUTPUT(OnIn) // can't use OnPressed because that is fired even if the master is disabled
	MASTER_INPUT(Unlock)
	BEGIN_KEYVALUES()
	RENAME_KEY(angles,movedir),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_func_button::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	m_szLetterName = NULL;

	const char *target = m_entity->ValueForKey("target");
	if (target[0])
	{
		TargetName *tn = LookupTargetNameFailOK(target);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
				if (entityList[i]->IsClassname("env_message"))
				{
					const char *message = entityList[i]->ValueForKey("message");
					m_szLetterName = EC_env_message::LetterName( message );
				}
				if (entityList[i]->IsClassname("trigger_relay"))
				{
					// TODO: Look for multipage letters too
				}
			}
		}
	}
}

void EC_func_button::Convert(void)
{
	if (m_szLetterName != NULL)
	{
		m_keyvalues = m_entity->m_chunk->m_keyvalues;
		m_entity->m_chunk->m_keyvalues = new KeyValues;

		AddKeyValue("classname", "func_letter");
		if (m_keyvalues->HasKey("targetname"))
			AddKeyValue("targetname", m_keyvalues->ValueForKey("targetname"));
		AddKeyValue("LetterName", m_szLetterName);
		if (m_keyvalues->HasKey("UseableString"))
			AddKeyValue("UseableString", m_keyvalues->ValueForKey("UseableString"));
		return;
	}

	BaseClass::Convert();
}

void EC_func_button::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 32 | 256);
	if (sf & 64) // sparks
		newsf |= 4096; // sparks
	newsf |= 1024; // use activates
#ifdef MASTER_LOCK
	// If the button has a master then start locked
	if (m_keyvalues->ValueForKey("master")[0])
		newsf |= 2048; // start locked
#endif
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_door )
	ON_INPUT(Toggle)
	TOGGLE_INPUT(Toggle)
	MASTER_INPUT(Unlock)
	BEST_OUTPUT(OnFullyOpen)
	BEGIN_KEYVALUES()
	RENAME_KEY(angles, movedir),
	XFORM_KEYVALUE(locked_sound, Handle_locked_sound),
	XFORM_KEYVALUE(movesnd, Handle_movesnd),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(stopsnd, Handle_stopsnd),
	XFORM_KEYVALUE(target, Handle_target),
	XFORM_KEYVALUE(unlocked_sound, Handle_unlocked_sound),
	// netname is target to call if the door is fully open/closed depending on
	// whether is *spawned* open or closed
	XFORM_KEYVALUE(netname, Handle_netname),
END_CONVERTER()

void EC_func_door::Convert(void)
{
	BaseClass::Convert();

	// If spawnflags wasn't specified then set the default
	if (!m_keyvalues->HasKey("spawnflags"))
	{
		SetSpawnflags(1024); // Touch Opens
	}

	// HL1: doors with a targetname would not open when touched.
	if (m_keyvalues->HasKey("targetname"))
	{
		UnsetSpawnflags(1024); // Touch Opens
	}
}

void EC_func_door::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 8 | 32 | 256 | 512);
	// HL1: "Use Only" flag means touch doesn't open.
	if (!(sf & 256)) // !Use Only
		newsf |= 1024; // Touch Opens
	SetSpawnflags(newsf);
}

void EC_func_door::Handle_target(KeyValue &kv)
{
	// Add *TWO* connections for each target matching our "target" entity name.
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				"OnFullyOpen",
				entityList[i],
				entityList[i]->m_converter->ToggleInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);

			m_entity->AddConnection(
				"OnFullyClosed",
				entityList[i],
				entityList[i]->m_converter->ToggleInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

void EC_func_door::Handle_netname(KeyValue &kv)
{
	const char *spawnflags = m_keyvalues->ValueForKey("spawnflags");
	int sf = Q_atoi(spawnflags);
	bool bStartOpen = (sf & 1) != 0;

	// Add one connection for each target matching our "netname" entity name.
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				bStartOpen ? "OnFullyClosed" : "OnFullyOpen",
				entityList[i],
				entityList[i]->m_converter->ToggleInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

void EC_func_door::Handle_locked_sound(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "Buttons.snd%d", n);
		AddKeyValue("locked_sound", string);
	}
}

void EC_func_door::Handle_movesnd(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "DoorSound.DoorMove%d", n);
		AddKeyValue("noise1", string);
	}
}

void EC_func_door::Handle_stopsnd(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "DoorSound.DoorStop%d", n);
		AddKeyValue("noise2", string);
	}
}

void EC_func_door::Handle_unlocked_sound(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "Buttons.snd%d", n);
		AddKeyValue("unlocked_sound", string);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_door_rotating )
	OFF_INPUT(Toggle)
	ON_INPUT(Toggle)
	TOGGLE_INPUT(Toggle)
	BEST_OUTPUT(OnFullyOpen)
	MASTER_INPUT(Unlock)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(netname, Handle_netname), // If an event is specified here, it will be activated when the door closes.
	XFORM_KEYVALUE(locked_sound, Handle_locked_sound),
	XFORM_KEYVALUE(movesnd, Handle_movesnd),
	XFORM_KEYVALUE(stopsnd, Handle_stopsnd),
	XFORM_KEYVALUE(unlocked_sound, Handle_unlocked_sound),

	// Hammer will set default spawnflags to "Touch opens" if no value was specified.
	// This will force a value of 0 if no value was specified.
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	ADD_KEYVALUE(spawnflags, 0),
END_CONVERTER()

void EC_func_door_rotating::Convert(void)
{
	BaseClass::Convert();

	// If spawnflags wasn't specified then set the default
	if (!m_keyvalues->HasKey("spawnflags"))
	{
		SetSpawnflags(1024); // Touch Opens
	}

	// HL1: doors with a targetname would not open when touched.
	if (m_keyvalues->HasKey("targetname"))
	{
		UnsetSpawnflags(1024); // Touch Opens
	}

}

void EC_func_door_rotating::Finalize(void)
{
#ifdef HUD_USEABLE
	// Some HOE doors don't have the trigger_multiple/env_message setup
	if (!m_entity->HasKey("UseableString"))
	{
		// FIXME: this catches a toilet seat too
		AddKeyValue("UseableString", "#OBJECT_DOOR");
	}
#endif
}

void EC_func_door_rotating::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 8 | 16 | 32 | 64 | 128 | 256 | 512);
#ifdef MASTER_LOCK
	// If the door has a master then start locked
	if (m_keyvalues->ValueForKey("master")[0])
		newsf |= 2048; // start locked
#endif
	if (!(sf & 256)) // !Use Only
		newsf |= 1024; // Touch Opens
	SetSpawnflags(newsf);
}

void EC_func_door_rotating::Handle_netname(KeyValue &kv)
{
	const char *spawnflags = m_keyvalues->ValueForKey("spawnflags");
	int sf = Q_atoi(spawnflags);
	bool bStartOpen = (sf & 1) != 0;

	// Add one connection for each target matching our "netname" entity name.
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				bStartOpen ? "OnFullyClosed" : "OnFullyOpen",
				entityList[i],
				entityList[i]->m_converter->ToggleInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

void EC_func_door_rotating::Handle_locked_sound(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "Buttons.snd%d", n);
		AddKeyValue("locked_sound", string);
	}
}

void EC_func_door_rotating::Handle_movesnd(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "DoorSound.DoorMove%d", n);
		AddKeyValue("noise1", string);
	}
}

void EC_func_door_rotating::Handle_stopsnd(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "DoorSound.DoorStop%d", n);
		AddKeyValue("noise2", string);
	}
}

void EC_func_door_rotating::Handle_unlocked_sound(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "Buttons.snd%d", n);
		AddKeyValue("unlocked_sound", string);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_illusionary )
	CONVERTED_CLASSNAME(func_brush)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(Solidity, 1), // never solid
	ADD_KEYVALUE(spawnflags, 2), // ignore player +USE
	REMOVE_KEYVALUE(skin), // FIXME: convert to a render property or something?
END_CONVERTER()

//-----------------------------------------------------------------------------
#if 1
BEGIN_CONVERTER( EC_func_ladder )
	CONVERTED_CLASSNAME(func_useableladder)
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_func_ladder::Convert(void)
{
	BaseClass::Convert();

	vmf_chunk *solid = m_entity->m_chunk->LookupChild("solid");
	if (!solid)
		return;

	vector min, max;
	GetBoundsOfSolid(solid, min, max);

	// top
	char szPoint[128];
	sprintf(szPoint, "%f %f %f", min.x + (max.x - min.x) / 2, min.y + (max.y - min.y) / 2, max.z);
	AddKeyValue("point0", szPoint);

	// bottom
	sprintf(szPoint, "%f %f %f", min.x + (max.x - min.x) / 2, min.y + (max.y - min.y) / 2, min.z);
	AddKeyValue("point1", szPoint);

	sprintf(szPoint, "%f %f %f", min.x + (max.x - min.x) / 2, min.y + (max.y - min.y) / 2, min.z + (max.z - min.z) / 2);
	SetKeyValue("origin", szPoint);

	m_entity->m_chunk->RemoveChild(solid);

	CEMsg("converted func_ladder to func_useableladder: you must update its endpoints\n");
}
#else
BEGIN_CONVERTER( EC_func_ladder )
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_func_ladder::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	vmf_chunk *solid = m_entity->m_chunk->LookupChild("solid");
	if (!solid)
		return;

	int count = 0;
	for (vmf_chunk *side = solid->LookupChild("side");
		side != 0;
		side = side->LookupNext("side"))
	{
		side->SetKeyValue("material", "tools/toolsinvisibleladder");
		count++;
	}
	if (count != 6)
		fprintf(stdout, "*** func_ladder expected six sides\n");
}
#endif

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_monsterclip )
	CONVERTED_CLASSNAME(func_brush)
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_func_monsterclip::Convert(void)
{
	BaseClass::Convert();

	const char *textures[] = {
		"", "tools/toolsnpcclip",
		NULL
	};
	m_entity->ReplaceTextures(textures);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_mortar_field )
	ON_INPUT(Shoot)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_pendulum )
	CONVERTED_CLASSNAME(func_brush) // brush so render properties work
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_func_pendulum::Convert(void)
{
	BaseClass::Convert();
	// Source doesn't have this, perhaps a rope?
	CEMsg("*** warning: func_pendulum converted to func_brush\n");
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_rotating )
	OFF_INPUT(Toggle)
	ON_INPUT(Toggle)
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(sounds, Handle_sounds),
	REMOVE_KEYVALUE(spawnorigin), // FIXME: different than "origin"?
	RENAME_KEY(speed, maxspeed),
END_CONVERTER()

void EC_func_rotating::Handle_sounds(KeyValue &kv)
{
	if ( kv.m_value[0] && Q_strcmp(kv.m_value, "0") )
	{
		int n = Q_atoi( kv.m_value );
		char string[32];
		sprintf(string, "Fan%d", n);
		AddKeyValue("message", string);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_rot_button )
	BEST_OUTPUT(OnIn)  // can't use OnPressed because that is fired even if the master is disabled
	MASTER_INPUT(Unlock)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	REMOVE_KEYVALUE(renderamt),
	REMOVE_KEYVALUE(renderfx),
	REMOVE_KEYVALUE(rendercolor),
	REMOVE_KEYVALUE(rendermode),
END_CONVERTER()

void EC_func_rot_button::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	// These should be converted to a func_button with the "Don't Move" spawnflag
	if (!m_entity->ValueForKey("distance")[0] || !Q_strcmp(m_entity->ValueForKey("distance"), "0"))
		m_entity->m_chunk->SetKeyValue("distance", "1");

	const char *textures[] = {
		"toolstrigger", "tools/toolsinvisible",
		NULL
	};
	entity->ReplaceTextures(textures);
}

void EC_func_rot_button::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 32 | 64 | 128 | 256);
	newsf |= 1024; // use activates
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_tank )
	TOGGLE_INPUT(Activate) // No Toggle?
	BEST_OUTPUT(OnFire) // "target" is name of ambient_generic to play bullet sound
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(bullet, Handle_bullet),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_func_tank::Handle_bullet(KeyValue &kv)
{
	const char *ammo = "None";

	switch (Q_atoi(kv.m_value))
	{
	case 0: ammo = "None"; break;
	case 1: ammo = "9mm"; break;
	case 2: ammo = "mp5"; break;
	case 3: ammo = "12mm"; break;
	}
	AddKeyValue("ammotype", ammo);
}

void EC_func_tank::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 4 | 8 | 16 | 32);
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_tankcontrols )
	CONVERTED_CLASSNAME(trigger_multiple)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(spawnflags, 1), // Clients
	XFORM_KEYVALUE(target, Handle_target), // func_tank we control
END_CONVERTER()

// In HL1, a func_tankcontrols specified the func_tank it controlled
// In Source, the func_tank has a "control_volume" keyvalue specifying a trigger
void EC_func_tankcontrols::Handle_target(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			entityList[i]->m_chunk->SetKeyValue("control_volume", m_entity->UniqueName());

			// rotate with parent
			// Doesn't work, func_tank faces east in map then rotates to 'angles', but
			// that initial rotation doesn't move entities parented to the func_tank
//			AddKeyValue("parentname", entityList[i]->UniqueName());
		}

		// Experimentation reveals that the trigger won't monitor the player touching it
		// if the trigger has no touch-related outputs.
//		m_entity->m_chunk->AddConnection("OnStartTouch", "no_such_entity", "no_such_input", "", "0", ONLY_ONCE_FALSE);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_tracktrain )
	TOGGLE_INPUT(Toggle)
	ON_INPUT(StartForward)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target),		// The "target" of a func_tracktrain is the first path_track
	REMOVE_KEYVALUE(sounds),	// FIXME: StartSound, StopSound
	RENAME_KEY(startspeed, speed), // WTF? Exactly reversed meanings in HL2
	RENAME_KEY(speed, startspeed), // WTF? Exactly reversed meanings in HL2
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_func_tracktrain::Convert(void)
{
	BaseClass::Convert();

#if 1
	CEMsg("*** make sure func_tracktrain is facing east, not west\n");
#else
	// This rotates the brushes properly, but not the textures
	if (Rotate180(m_entity))
		fprintf(stdout, "ROTATED func_tracktrain 180 degrees\n");
	else
		fprintf(stdout, "*** failed to rotate func_tracktrain 180 degrees\n");
#endif
}

void EC_func_tracktrain::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 8);
	newsf |= 0x80; // HL1 train -> SOLID_BSP
	newsf |= 512; // unblockable by player
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_train )// FIXME: this entity is deprecated in Source I believe
	ON_INPUT(Start)
	OFF_INPUT(Stop)
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target),		// The "target" of a func_train is the first path_corner
	XFORM_KEYVALUE(movesnd, Handle_movesnd),
	XFORM_KEYVALUE(stopsnd, Handle_stopsnd),
END_CONVERTER()

void EC_func_train::Handle_movesnd(KeyValue &kv)
{
	static const char *movesnd[] = {
		"Plat.BigElev1",
		"Plat.BigElev2",
		"Plat.TechElev1",
		"Plat.TechElev2",
		"Plat.TechElev3",
		"Plat.FreightElev1",
		"Plat.FreightElev2",
		"Plat.HeavyElev",
		"Plat.RackElev",
		"Plat.RailElev",
		"Plat.SqueakElev",
		"Plat.OddElev1",
		"Plat.OddElev2",
	};
	if ( kv.m_value[0] )
	{
		int n = Q_atoi( kv.m_value );
		if (n >= 1 && n <= 13)
			AddKeyValue("noise1", movesnd[n - 1]);
	}
}

void EC_func_train::Handle_stopsnd(KeyValue &kv)
{
	static const char *stopsnd[] = {
		"Plat.BigElevStop1",
		"Plat.BigElevStop2",
		"Plat.FreightElevStop",
		"Plat.HeavyElevStop",
		"Plat.RackStop",
		"Plat.RailStop",
		"Plat.SqueakStop",
		"Plat.QuickStop",
	};
	if ( kv.m_value[0] )
	{
		int n = Q_atoi( kv.m_value );
		if (n >= 1 && n <= 8)
			AddKeyValue("noise2", stopsnd[n - 1]);
	}
}

//-----------------------------------------------------------------------------
#if 1
BEGIN_CONVERTER( EC_func_wall )
	BEGIN_KEYVALUES()
END_CONVERTER()
#else
BEGIN_CONVERTER( EC_func_wall )
	CONVERTED_CLASSNAME(func_brush) // FIXME: func_detail has no inputs
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_func_wall::Convert()
{
	m_keyvalues = m_entity->m_chunk->m_keyvalues;
	m_entity->m_chunk->m_keyvalues = new KeyValues;

	// Turn a func_wall with no targetname or render properties into a func_detail.
	// Turn a func_wall with a targetname or render properties into a func_brush.

	// goldsrc func_wall:
	//   targetname, globalname, renderfx, rendermode, renderamt, rendercolor

	const char *targetname = m_keyvalues->ValueForKey("targetname");
	const char *globalname = m_keyvalues->ValueForKey("globalname");
	const char *renderfx = m_keyvalues->ValueForKey("renderfx");
	const char *rendermode = m_keyvalues->ValueForKey("rendermode");
	const char *renderamt = m_keyvalues->ValueForKey("renderamt");
	const char *rendercolor = m_keyvalues->ValueForKey("rendercolor");

	// source func_detail: mindxlevel, maxdxlevel
	// source func_brush: same as goldsrc func_wall + extra keyvalues

	if (targetname[0] ||
		globalname[0] ||
		Q_strcmp(renderfx, "0") ||
		Q_strcmp(rendermode, "0"))
	{
		AddKeyValue("classname", "func_brush");
		if (targetname[0])
			Handle_targetname(*m_keyvalues->Find("targetname"));
		AddKeyValue("globalname", globalname); // FIXME: does this need to be unique?
		AddKeyValue("renderfx", renderfx);
		AddKeyValue("rendermode", rendermode);
		AddKeyValue("renderamt", renderamt);
		AddKeyValue("spawnflags", "2"); // ignore player +USE

		if (rendercolor[0])
			Handle_rendercolor(*m_keyvalues->Find("rendercolor"));
#if 0
		// This next bit is from MapFOOL, the "RenderColor bug"
		if ((!renderfx[0] || !Q_strcmp(renderfx, "0")) &&
			(!rendermode[0] || !Q_strcmp(rendermode, "0")) &&
			!Q_strcmp(rendercolor, "0 0 0"))
		{
			AddKeyValue("rendercolor", "255 255 255");
			fprintf(stdout, "converted func_wall's rendercolor 0,0,0 -> 255,255,255\n");
		}
		else
			AddKeyValue("rendercolor", rendercolor);
#endif
//		fprintf(stdout, "converted interesting func_wall -> func_brush\n");
	}
	else
	{
		AddKeyValue("classname", "func_detail");
		fprintf(stdout, "converted boring func_wall -> func_detail\n");
	}
}
#endif

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_wall_toggle )
	OFF_INPUT(Toggle) // always toggle
	TOGGLE_INPUT(Toggle) // not listed in Wiki
	BEGIN_KEYVALUES()
//	ADD_KEYVALUE(Solidity, 0) // becomes a func_brush with Solidity==Toggle
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_func_water ) // implemented as func_door in Source
	OFF_INPUT(Toggle)
	ON_INPUT(Toggle)
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_infodecal )
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_infodecal::Convert(void)
{
	BaseClass::Convert();

	if ( !m_entity->HasKey("LowPriority") )
		AddKeyValue("LowPriority", "0");

	const char *texture = m_entity->ValueForKey("texture");
	Replace(texture, "signrbhangar", "signbhangar");
	Replace(texture, "rwalktheearth", "walktheearth");
	Replace(texture, "signrauxgen", "signauxgen");
	Replace(texture, "signrb", "signb");
	Replace(texture, "signrbase", "signbase");
	Replace(texture, "signrbb", "signbb");
	Replace(texture, "signrbbase", "signbbase");
	Replace(texture, "signrbhangar", "signbhangar");
	Replace(texture, "signrboathouse", "signboathouse");
	Replace(texture, "signrhangar", "signhangar");
	Replace(texture, "signrhangars", "signhangars");
	Replace(texture, "signrmedical", "signmedical");
	Replace(texture, "signrordstor", "signordstor");
	Replace(texture, "signrsupplies", "signsupplies");
	Replace(texture, "villager", "village");
}

void EC_infodecal::Replace(const char *texture, const char *oldName, const char *newName)
{
	const char *p = strstr(texture, oldName);
	if (p && !p[strlen(oldName)])
		strcpy((char *) p, newName); // this only works if the string gets shorter
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_light )
	OFF_INPUT(TurnOff)
	ON_INPUT(TurnOn)
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monstermaker )
	CONVERTED_CLASSNAME(npc_maker)
	TOGGLE_INPUT(Toggle) // FIXME: if the flag "4" is specified. then the best input is "Spawn"
	BEST_OUTPUT(OnSpawnNPC)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(childtarget),
	RENAME_KEY(m_imaxlivechildren, MaxLiveChildren),
	RENAME_KEY(monstercount, MaxNPCCount),
	RENAME_KEY(netname, NPCTargetname),
	RENAME_KEY(delay, SpawnFrequency),
	CLASSNAME_KEYVALUE(monstertype, NPCType),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(TriggerTarget, Handle_TriggerTarget),
	REMOVE_KEYVALUE(TriggerCondition),
END_CONVERTER()

void EC_monstermaker::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	// If this monstermaker assigns a name to a spawned monster then that
	// monster will probably receive inputs from other entities. So we must
	// create a dummy monster (that isn't saved to the VMF) with that name.
	const char *netname = m_entity->ValueForKey("netname");
	if (netname[0] && Q_strcmp(netname, "0"))
	{
		const char *monstertype = m_entity->ValueForKey("monstertype");
		const char *classname = ConvertedClassname(monstertype);
		vmf_entity::CreateDummy(monstertype /*classname*/, netname);

		// HOE has a "childtarget" field which is used to set the "target" field of
		// a spawned Huey, which is the path_track the Huey starts on.
		const char *childtarget = m_entity->ValueForKey("childtarget");
		if (childtarget[0] && Q_strcmp(childtarget, "0") && !Q_stricmp(monstertype, "monster_huey"))
		{
			m_entity->m_chunk->AddConnection("OnSpawnNPC", netname, "SetTrack", childtarget, "0", ONLY_ONCE_FALSE);
			
			EC_monster_huey::ConvertPathCornerToPathTrack(netname, childtarget);
		}


	}
}

void EC_monstermaker::Handle_spawnflags(KeyValue &kv)
{
	// FIXME: other spawnflags
	int sf = atol(kv.m_value);
	AddKeyValue("StartDisabled", (sf & 1) ? "0" : "1");
}

// FIXME: just make a template NPC
void EC_monstermaker::Handle_TriggerTarget(KeyValue &kv)
{
	// HOE adds TriggerTarget and TriggerCondition to the spawned entity.
	// Used so a spawned Huey can trigger another entity when it dies.
	const char *TriggerTarget = kv.m_value;
	const char *TriggerCondition = m_keyvalues->ValueForKey("TriggerCondition");
	const char *netname = m_keyvalues->ValueForKey("netname");
	if (TriggerTarget[0] && TriggerCondition[0])
	{
		TargetName *tn = LookupTargetNameFailOK(TriggerTarget);
		if (tn == NULL)
			return;

		const TargetName::EntityListType &entityList = tn->GetEntityList();

		char *output;
		switch (atoi(TriggerCondition))
		{
		case 2: output = "OnDamaged"; break;
		case 4: output = "OnDeath"; break;
		default:
			fprintf(stdout, "*** WARNING: unhandled TriggerCondition %d in EC_monstermaker::Init\n", atoi(TriggerCondition));
			output = "OnDeath";
			break;
		}

		char outputStr[128];
		for (int i = 0; i < entityList.Count(); i++)
		{
			const char *inputToTT = entityList[i]->m_converter->ToggleInput(NULL/*spawned entity here*/);
			sprintf(outputStr, "%s %s:%s:%s:%s:%s", output, entityList[i]->UniqueName(), inputToTT, "", "0", "-1");
			m_entity->m_chunk->AddConnection("OnSpawnNPC", netname, "AddOutput", outputStr, "0", ONLY_ONCE_FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_xyz )
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(body),
	REMOVE_KEYVALUE(TriggerCondition),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(TriggerTarget, Handle_TriggerTarget),
END_CONVERTER()

void EC_monster_xyz::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_xyz::Handle_TriggerTarget(KeyValue &kv)
{
	TargetName *tn = LookupTargetNameFailOK(kv.m_value);
	if (!tn)
		return;

	const TargetName::EntityListType &entityList = tn->GetEntityList();

	 //0 - No Trigger (default)
	 //1 - See Player, Mad at Player
	 //2 - Take Damage
	 //3 - 50% Health Remaining
	 //4 - Death
	 //7 - Hear World
	 //8 - Hear Player
	 //9 - Hear Combat
	 //10 - See Player Unconditional
	 //11 - See Player, Not In Combat
	char *output;
	const char *TriggerCondition = m_keyvalues->ValueForKey("TriggerCondition");
	switch (atoi(TriggerCondition))
	{
	case 2: output = "OnDamaged"; break;
	case 4: output = "OnDeath"; break;
	default:
		fprintf(stdout, "*** WARNING: unhandled TriggerCondition %d for monster\n", atoi(TriggerCondition));
		output = "OnDeath";
		break;
	}

	for (int i = 0; i < entityList.Count(); i++)
	{
		m_entity->AddConnection(
			output,
			entityList[i],
			entityList[i]->m_converter->ToggleInput(m_entity),
			"",
			m_keyvalues->ValueForKey("delay", "0"), // FIXME: or always "0"?
			ONLY_ONCE_FALSE);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_alien_controller )
	CONVERTED_CLASSNAME(npc_spx_flyer)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_alien_slave )
	CONVERTED_CLASSNAME(npc_vortigaunt)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_barnacle )
	CONVERTED_CLASSNAME(npc_barnacle)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_barney )
	CONVERTED_CLASSNAME(npc_barney)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_barneyzombie )
	CONVERTED_CLASSNAME(npc_barneyzombie)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_bullchicken )
	CONVERTED_CLASSNAME(npc_bullsquid)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_charlie )
	CONVERTED_CLASSNAME(npc_charlie)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_charlie::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	if (sf & 32) // squad leader
		newsf |= (1 << 17); // squad leader
	if (sf & 64) // grenades
		newsf |= (1 << 18); // grenades
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_charlie::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_ak47"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_ak47";
	else if (!Q_strcmp(kv.m_value, "1"))
		weapon = "weapon_rpg7";
	else if (!Q_strcmp(kv.m_value, "2"))
		return; // None
	
	AddKeyValue("additionalequipment", weapon);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_charlie_dead )
	CONVERTED_CLASSNAME(npc_charlie_dead)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_chumtoad )
	CONVERTED_CLASSNAME(npc_chumtoad)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_furniture)
	CONVERTED_CLASSNAME(npc_furniture)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_generic )
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_gorilla )
	CONVERTED_CLASSNAME(npc_gorilla)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_grunt_medic_repel )
	CONVERTED_CLASSNAME(npc_grunt_medic)
	TOGGLE_INPUT(BeginRappel)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(sleepstate, 3), // wait for "Wake" input
	ADD_KEYVALUE(waitingtorappel, 1),
END_CONVERTER()

bool EC_monster_grunt_medic_repel::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// Whichever entity tells us to begin rappeling must also wake us
	if (!Q_stricmp(input, "BeginRappel"))
	{
		caller->AddConnection(
			output,
			m_entity,
			"Wake",
			"",
			delay,
			onlyOnce);
	}

	return false;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_grunt_repel )
	CONVERTED_CLASSNAME(npc_human_grunt)
	TOGGLE_INPUT(BeginRappel)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(sleepstate, 3), // wait for "Wake" input
	ADD_KEYVALUE(waitingtorappel, 1),
END_CONVERTER()

bool EC_monster_grunt_repel::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// Whichever entity tells us to begin rappeling must also wake us
	if (!Q_stricmp(input, "BeginRappel"))
	{
		caller->AddConnection(
			output,
			m_entity,
			"Wake",
			"",
			delay,
			onlyOnce);
	}

	return false;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_headcrab )
	CONVERTED_CLASSNAME(npc_spx_baby)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_huey )
	CONVERTED_CLASSNAME(npc_huey)
	TOGGLE_INPUT(Wake) // namf5 USEs a npc_maker-spawned huey to start it
	ON_INPUT(Activate)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target), // the first path to go to
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_monster_huey::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	const char *pathName = entity->ValueForKey("target");
#if 1
	if (!ConvertPathCornerToPathTrack(entity->UniqueName(), pathName))
	{/*m_entity->m_chunk->SetKeyValue("target", "")*/;}
#else
	vmf_entity *pathEnt, *pathEnt0 = 0;
	pathEnt = FindEntity(NULL, pathName);

	if (pathEnt)
	{
	// namd0 uses scripted_sequence not path_corner!!!
		if (Q_strcmp(pathEnt->m_chunk->ValueForKey("classname"), "path_corner"))
			entity->m_chunk->SetKeyValue("target", "");
		return;
	}

	while (pathEnt)
	{
		// Check for circular path
		if (pathEnt == pathEnt0)
			break;
		if (!pathEnt0)
			pathEnt0 = pathEnt;

		// Change path_corner to path_track
		pathEnt->m_chunk->SetKeyValue("classname", "path_track");
		fprintf(stdout, "converted path_corner \"%s\" into path_track\n", pathName);

		// In HL1 the huey has a chance to deploy troops at any speed==0 path_corner.
		if (!Q_strcmp(pathEnt->ValueForKey("speed"), "0"))
		{
			pathEnt->AddConnection("OnPass", m_entity, "Deploy", pathName, "0", ONLY_ONCE_FALSE);
		}

		// Find the next path_corner
		pathName = pathEnt->ValueForKey("target");
		pathEnt = FindEntity(NULL, pathName);
	}
#endif
}

void EC_monster_huey::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = 0;
	if (sf & 4) // wait for trigger
		newsf |= 0x40; // wait for trigger
	if (sf & 64) // passenger
		newsf |= 128; // passenger
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

bool EC_monster_huey::ConvertPathCornerToPathTrack(const char *hueyName, const char *pathName)
{
	vmf_entity *pathEnt;
	pathEnt = FindEntity(NULL, pathName);

	if (pathEnt)
	{
		// namd0 uses scripted_sequence not path_corner!!!
		if (Q_stricmp(pathEnt->m_chunk->ValueForKey("classname"), "path_corner"))
		{
			return false;
		}
	}

	CUtlVector<vmf_entity *> seen;
	while (pathEnt)
	{
		// Check for circular path
		if ( seen.HasElement( pathEnt ) )
			break;
		seen.AddToTail( pathEnt );

		// Change path_corner to path_track
		pathEnt->m_chunk->SetKeyValue("classname", "path_track");
		fprintf(stdout, "converted path_corner \"%s\" into path_track\n", pathName);

		// In HL1 the huey has a chance to deploy troops at any speed==0 path_corner.
		if (!Q_strcmp(pathEnt->ValueForKey("speed"), "0"))
		{
			pathEnt->m_chunk->AddConnection("OnPass", hueyName, "Deploy", pathName, "0", ONLY_ONCE_FALSE);
		}

		// Find the next path_corner
		pathName = pathEnt->ValueForKey("target");
		pathEnt = FindEntity(NULL, pathName);
	}
	return true;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_human_grunt )
	CONVERTED_CLASSNAME(npc_human_grunt)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_human_grunt::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	if (sf & 8) // friendly
		newsf |= (1 << 16); // friendly
	if (sf & 32) // squad leader
		newsf |= (1 << 17); // squad leader
	if (sf & 64) // grenades
		newsf |= (1 << 18); // grenades
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_human_grunt::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_m16"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_m16";
	else if (!Q_strcmp(kv.m_value, "1"))
		weapon = "weapon_870";
	else if (!Q_strcmp(kv.m_value, "2"))
		weapon = "weapon_m79";
	
	AddKeyValue("additionalequipment", weapon);
}


//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_hgrunt_dead )
	CONVERTED_CLASSNAME(npc_hgrunt_dead)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_human_grunt_medic )
	CONVERTED_CLASSNAME(npc_grunt_medic)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_human_grunt_medic::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	if (sf & 8) // friendly
		newsf |= (1 << 16); // friendly
	if (sf & 32) // squad leader
		newsf |= (1 << 17); // squad leader
	if (sf & 64) // grenades
		newsf |= (1 << 18); // grenades
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_human_grunt_medic::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_colt1911A1"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_colt1911A1";
	
	AddKeyValue("additionalequipment", weapon);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_houndeye )
	CONVERTED_CLASSNAME(npc_houndeye)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname),
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_kophyaeger )
	CONVERTED_CLASSNAME(npc_kophyaeger)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_kophyaeger_adult )
	CONVERTED_CLASSNAME(npc_kophyaeger_adult)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_kurtz )
	CONVERTED_CLASSNAME(npc_kurtz)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_mikeforce )
	CONVERTED_CLASSNAME(npc_mikeforce)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_mikeforce::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	if (sf & 32) // squad leader
		newsf |= (1 << 17); // squad leader
	if (sf & 64) // grenades
		newsf |= (1 << 18); // grenades
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_mikeforce::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_m16"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_m16";
	else if (!Q_strcmp(kv.m_value, "1"))
		weapon = "weapon_870";
	else if (!Q_strcmp(kv.m_value, "2"))
		weapon = "weapon_m60";
	else if (!Q_strcmp(kv.m_value, "3"))
		return; // None
	
	AddKeyValue("additionalequipment", weapon);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_mikeforce_dead )
	CONVERTED_CLASSNAME(npc_mikeforce_dead)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_mikeforce_repel )
	CONVERTED_CLASSNAME(npc_mikeforce)
	TOGGLE_INPUT(BeginRappel)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(sleepstate, 3), // wait for "Wake" input
	ADD_KEYVALUE(waitingtorappel, 1),
END_CONVERTER()

bool EC_monster_mikeforce_repel::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// Whichever entity tells us to begin rappeling must also wake us
	if (!Q_stricmp(input, "BeginRappel"))
	{
		caller->AddConnection(
			output,
			m_entity,
			"Wake",
			"",
			delay,
			onlyOnce);
	}

	return false;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_mikeforce_medic )
	CONVERTED_CLASSNAME(npc_mikeforce_medic)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_mikeforce_medic::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	if (sf & 32) // squad leader
		newsf |= (1 << 17); // squad leader
	if (sf & 64) // grenades
		newsf |= (1 << 18); // grenades
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_mikeforce_medic::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_colt1911A1"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_colt1911A1";
	
	AddKeyValue("additionalequipment", weapon);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_mikeforce_medic_repel )
	CONVERTED_CLASSNAME(npc_mikeforce_medic)
	TOGGLE_INPUT(BeginRappel)
	BEGIN_KEYVALUES()
	ADD_KEYVALUE(sleepstate, 3), // wait for "Wake" input
	ADD_KEYVALUE(waitingtorappel, 1),
END_CONVERTER()

bool EC_monster_mikeforce_medic_repel::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// Whichever entity tells us to begin rappeling must also wake us
	if (!Q_stricmp(input, "BeginRappel"))
	{
		caller->AddConnection(
			output,
			m_entity,
			"Wake",
			"",
			delay,
			onlyOnce);
	}

	return false;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_peasant )
	CONVERTED_CLASSNAME(npc_peasant)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_peasant_dead )
	CONVERTED_CLASSNAME(npc_peasant_dead)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_scientist_dead )
	CONVERTED_CLASSNAME(npc_scientist_dead)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_sog )
	CONVERTED_CLASSNAME(npc_sog)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_superzombie )
	CONVERTED_CLASSNAME(npc_superzombie)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	RENAME_KEY(netname, squadname),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
	XFORM_KEYVALUE(weapons, Handle_weapons),
END_CONVERTER()

void EC_monster_superzombie::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = 0;
	if (sf & 8) // friendly
		newsf |= (1 << 16); // friendly
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

void EC_monster_superzombie::Handle_weapons(KeyValue &kv)
{
	char *weapon = "weapon_m16"; // in case of error

	if (!Q_strcmp(kv.m_value, "0"))
		weapon = "weapon_m16";
	else if (!Q_strcmp(kv.m_value, "1"))
		weapon = "weapon_870";
	else if (!Q_strcmp(kv.m_value, "2"))
		weapon = "weapon_m60";
	else if (!Q_strcmp(kv.m_value, "3"))
		weapon = "weapon_rpg7";
	else if (!Q_strcmp(kv.m_value, "4"))
		return; // None
	
	AddKeyValue("additionalequipment", weapon);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_tripmine )
	CONVERTED_CLASSNAME(npc_tripmine)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_vortigaunt )
	CONVERTED_CLASSNAME(npc_vortigaunt)
	BEGIN_KEYVALUES()
	RENAME_KEY(netname, squadname)
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_monster_zombie )
	CONVERTED_CLASSNAME(npc_spx)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(body),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_monster_zombie::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = sf & (1 | 2 | 128 | 512);
	if (sf & 4) {}; // FIXME: MonsterClip
	if (sf & 8) // friendly
		newsf |= (1 << 16); // friendly
	if (sf & 16) {}; // FIXME: Prisoner
	if (sf & 256) {}; // FIXME: Pre-Disaster
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_multi_manager )
	CONVERTED_CLASSNAME(logic_relay)
	TOGGLE_INPUT(Trigger)
	ON_INPUT(Trigger)
	BEST_OUTPUT(OnTrigger)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(spawnflags), // so our Handle_unknown doesn't see it
END_CONVERTER()

void EC_multi_manager::Handle_unknown(KeyValue &kv)
{
	// Add one connection for each target. The targets are stored as
	// extra key/value pairs.

	// Remove trailing #1, #2 etc
	char stripped[128], *p;
	Q_strncpy(stripped, kv.m_key, sizeof(stripped));
	if ((p = strchr(stripped, '#')))
		*p = '\0';
	
	TargetName *tn = LookupTargetName(stripped);
	const TargetName::EntityListType &entityList = tn->GetEntityList();
	for (int i = 0; i < entityList.Count(); i++)
	{
		m_entity->AddConnection(
			entityList[i],
			"",
			kv.m_value, // delay
			ONLY_ONCE_FALSE);
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_multisource )
	CONVERTED_CLASSNAME(logic_and)
	OFF_INPUT(FIXME) // see CallersFixup_multisource
	TOGGLE_INPUT(FIXME) // see CallersFixup_multisource
	ON_INPUT(FIXME)
	BEST_OUTPUT(OnAllTrue)
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_multisource::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	CallersFixup_multisource *fixup = new CallersFixup_multisource;
	fixup->Init(entity);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_path_corner )
	BEST_OUTPUT(OnPass)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target), // The "target" of a path_corner is the next path.
	XFORM_KEYVALUE(message, Handle_message), // "message" key holds name of target to activate when passing
END_CONVERTER()

void EC_path_corner::Handle_message(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				entityList[i],
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE); // only once
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_path_track )
	TOGGLE_INPUT(ToggleAlternatePath) // Branch Path - this is the targetname of a secondary path_track.
									// If a path_track has a Branch Path value, and it is triggered,
									// a func_tracktrain will go to the path_track specified as the
									// Branch Path rather than the Next Stop Target.
	BEST_OUTPUT(OnPass)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target), // The "target" of a path_track is the next path.
	XFORM_KEYVALUE(message, Handle_message), // holds name of target to activate when passing
	XFORM_KEYVALUE(netname, Handle_message), // holds name of target to activate when passing *if this is a dead end*
											 // FIXME: how to tell if this is a dead end?
END_CONVERTER()

void EC_path_track::Handle_message(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (!tn) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				entityList[i],
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE); // only once
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_player_loadsaved )
	TOGGLE_INPUT(Reload)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(message), // Message to display before reloading
	REMOVE_KEYVALUE(messagetime), // Delay before displaying message
//	XFORM_KEYVALUE(message, Handle_message), // Message to display before reloading
END_CONVERTER()

void EC_player_loadsaved::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	m_env_message = 0;
	const char *message = m_entity->m_chunk->ValueForKey("message");
	if (message[0])
	{
		// FIXME: add a unique "id" keyvalue to env
		m_env_message = m_entity->AddEnvMessage(message);
	}
}

#if 0
void EC_player_loadsaved::Handle_message(KeyValue &kv)
{
	// Tell callers to this entity to also call a new env_message
	if (kv.m_value[0])
	{
		CallersFixup_player_loadsaved *fixup = new CallersFixup_player_loadsaved;
		fixup->Init(m_entity);

		// FIXME: add a unique "id" keyvalue to env
		fixup->m_env_message = m_entity->AddEnvMessage(kv.m_value);
		fixup->m_messagetime = m_keyvalues->ValueForKey("messagetime");
	}
}
#endif

bool EC_player_loadsaved::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	if (!m_env_message)
		return false;

	// Tell callers to this entity to also call a new env_message
	caller->AddConnection(
		output,
		m_env_message,
		m_env_message->m_converter->ToggleInput(m_entity),
		"", // param
		m_keyvalues ? m_keyvalues->ValueForKey("messagetime", "0") : "0",
		onlyOnce);

	return false;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_player_weaponstrip )
	TOGGLE_INPUT(Strip)
	BEGIN_KEYVALUES()
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_scripted_sentence )
	ON_INPUT(BeginSentence)
	TOGGLE_INPUT(BeginSentence)
	BEST_OUTPUT(OnBeginSentence) // In HL1 "delay" is the time since the sentence began
								 // before the output is fired. HOE typically sets this
								 // to the sentence duration. In Source "delay" is additional
								 // time after the sentence finishes.
	BEGIN_KEYVALUES()
	ENTITY_VALUE(entity),
	REMOVE_KEYVALUE(duration), // duration is in sentences.txt now
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_scripted_sequence )
	ON_INPUT(BeginSequence)
	TOGGLE_INPUT(BeginSequence)
	BEST_OUTPUT(OnEndSequence)
	BEGIN_KEYVALUES()
	ENTITY_VALUE(m_iszEntity),
	XFORM_KEYVALUE(target, Handle_target), // entity to call when sequence ends
END_CONVERTER()

vmf_entity *EC_scripted_sequence::s_logicAuto = 0;

void EC_scripted_sequence::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	m_bLoopWithTriggerRelay = false;
	m_bLoopWithSelf = false;
	m_bStartOnSpawn = false;
	m_bWaitForInput = true;
	m_bTaskEnableScript = false;
	m_loopEntity = NULL;
	m_nextSS = NULL;

	// If the target of this SS is a trigger_relay, and the target of the
	// trigger_relay is this SS, it means we are supposed to loop until the
	// trigger_relay is killed.
	const char *targetname = m_entity->ValueForKey("targetname");
	const char *target = m_entity->ValueForKey("target");
	const char *m_iszEntity = m_entity->ValueForKey("m_iszEntity");
	if (targetname[0] && target[0])
	{
		// Some SS target themselves to loop forever.
		if (!Q_stricmp(targetname, target))
		{
			m_bLoopWithSelf = true;
		}
		else
		{
			TargetName *tn = LookupTargetNameFailOK(target);
			if (tn != NULL)
			{
				const TargetName::EntityListType &entityList = tn->GetEntityList();
				for (int i = 0; i < entityList.Count(); i++)
				{
					if (!Q_stricmp(entityList[i]->Classname(), "trigger_relay"))
					{
						const char *targetOfTR = entityList[i]->ValueForKey("target");
						if (!Q_stricmp(targetOfTR, targetname))
						{
							CEMsg("scripted_sequence %s loops with trigger_relay %s\n", targetname, target);
							m_bLoopWithTriggerRelay = true;
							m_loopEntity = entityList[i];
							break;
						}
					}

					// Determine if this SS starts another when it ends. In that case set the
					// m_iszNextScript field rather than adding an output.
					if (!Q_stricmp(entityList[i]->Classname(), "scripted_sequence"))
					{
						// It is possible that m_iszEntity is a classname not a targetname.
						// We don't have to worry about the classname being converted yet
						// because this Init() method is called before any conversions.
						const char *m_iszEntity2 = entityList[i]->ValueForKey("m_iszEntity");
						if (!Q_stricmp(m_iszEntity, m_iszEntity2))
						{
							CEMsg("scripted_sequence '%s' next SS is '%s'\n", targetname, target);
							m_nextSS = entityList[i];
						}
						else
						{
							CEMsg("*** WARNING scripted_sequence %s(%s) calls SS %s(%s) \n", targetname, m_iszEntity, target, m_iszEntity2);
						}
					}
				}
			}
		}
	}

	// In HL1 TASK_ENABLE_SCRIPT is called for moveto=walk or moveto=run only.
	// In HL2 TASK_ENABLE_SCRIPT is called for any value of moveto.
	const char *m_fMoveTo = m_entity->ValueForKey("m_fMoveTo");
	if (!Q_strcmp(m_fMoveTo, "1") || !Q_strcmp(m_fMoveTo, "2"))
		m_bTaskEnableScript = true;

	// In HL1 TASK_ENABLE_SCRIPT did not check if the SS was waiting for input before
	// starting. A SS waits for input if targetname!="". So in HL1 if a SS had a targetname
	// and an m_iszIdle animation it would not wait for +USE if moveto=walk or moveto=run.
	if (m_bTaskEnableScript && targetname[0] && m_entity->ValueForKey("m_iszIdle")[0])
		m_bWaitForInput = false;

	// In HL1 and HL2 a SS with no targetname will begin the action sequence once the
	// NPC reaches the SS position.
	// But in HL1 if moveto=instantaneous then the SS will wait for +USE
	if (!targetname[0] && Q_strcmp(m_fMoveTo, "4"))
		m_bWaitForInput = false;

	// HL1: a SS with targetname=="" or m_iszIdle!="" will start on spawn.
	// HL2: a SS with targetname=="" or SF_SCRIPT_START_ON_SPAWN will start on spawn.
	// NOTE: this doesn't imply the action sequence begins, only that the NPC will be
	// possessed (if in radius) on map spawn.
	if (!targetname[0] || m_entity->ValueForKey("m_iszIdle")[0])
		m_bStartOnSpawn = true;
}

void EC_scripted_sequence::Convert(void)
{
	const char *targetname = m_entity->ValueForKey("targetname");
#if 0
	if (m_bLoopWithSelf || m_bLoopWithTriggerRelay)
	{
		SetKeyValue("target", ""); // FIXME: make sure "target" refers only to this SS or trigger_relay
	}
#endif
	BaseClass::Convert();


	// In both versions, the SS must be used/inputted to begin the action sequence if
	// targetname was specified.
	// A scripted_sequence with no targetname will start on spawn. Add the start-on-spawn
	// spawnflag just in case we give this sequence a name.

	if (m_bStartOnSpawn)
	{
		const char *spawnflags = m_entity->ValueForKey("spawnflags");
		int sf = Q_atoi(spawnflags);
		sf |= 16; // start on spawn
		char s[32]; sprintf(s, "%d", sf);
		SetKeyValue("spawnflags", s);
	}

	// Lots of scripted sequences in HOE don't reach their target NPC.
	// So set the search radius to 0==everywhere if there was a targetname.
	// (If there was a targetname, then BeginSequence should be called sometime
	// so we don't need to wait for the target to enter our radius.)
	// If the radius is not the default radius of 512, assume the mapmaker got it right.
#if 1
	if (targetname[0] && !Q_strcmp(m_entity->ValueForKey("m_flRadius"), "512"))
		SetKeyValue("m_flRadius", "0");
#else
	const char *repeat = m_entity->ValueForKey("m_flRepeat");
	if (!Q_strcmp(repeat, "0"))
		SetKeyValue("m_flRadius", "0");
#endif

	// A scripted_sequence with no name will be triggered on spawn, but in Source
	// that doesn't play the action animation, only moves the NPC. In HL1 the
	// idle animation will start playing after the move.
}

void EC_scripted_sequence::Finalize(void)
{
	bool hasBeginSequence = false;
	for (int i = 0; i < m_entity->m_callers.Count(); i++)
	{
		vmf_entity *caller = m_entity->m_callers[i];
		if (caller->FindConnection(m_entity, "BeginSequence") >= 0)
		{
			hasBeginSequence = true;
			break;
		}
	}

	// If targetname=="" the action sequence will begin when the NPC reaches the SS.
	// If targetname!="" BeginSequence must be called.
#if 0
	if (!hasBeginSequence || m_entity->ValueForKey("m_iszIdle")[0])
	{
		const char *spawnflags = m_entity->ValueForKey("spawnflags");
		int sf = Q_atoi(spawnflags);
		sf |= 16; // start on spawn
		char s[32]; sprintf(s, "%d", sf);
		SetKeyValue("spawnflags", s);
	}
#endif

	const char *m_iszIdle = m_entity->ValueForKey("m_iszIdle");
	const char *m_iszPlay = m_entity->ValueForKey("m_iszPlay");

	bool bLoop = false;
#if 0
	if (!hasBeginSequence && !m_iszPlay[0] && m_iszIdle[0])
	{
		bLoop = true;
	}
#endif

	if (m_bLoopWithSelf || m_bLoopWithTriggerRelay || bLoop)
	{
		if (!m_iszPlay[0])
		{
			SetKeyValue("m_iszPlay", m_iszIdle[0] ? m_iszIdle : "ACT_IDLE");
			SetKeyValue("m_iszIdle", "");
		}
		SetKeyValue("m_bLoopActionSequence", "1");
	}

	// In HL1 a SS with targetname=="" and moveto=instantaneous would not play
	// the action sequence until +USE. So force targetname!="" otherwise HL2 will
	// not wait for BeginSequence.
	if (m_bWaitForInput)
	{
		if (!m_entity->ValueForKey("targetname")[0])
			SetKeyValue("targetname", m_entity->UniqueName());
	}
	// If we are supposed to begin the sequence on spawn
	// then force it to begin when the map spawns by calling BeginSequence from a
	// logic_auto entity.
	else
	{
		CallBeginSequenceFromLogicAuto();
	}

	// Since no other entity tells this one to begin, force the start-on-spawn flag.

	// If a SS has m_iszIdle!="" then the animation will loop until the
	// SS receives BeginSequence.
	// If we have a pre-action idle animation, assume we are supposed to loop in that
	// animation (since we will never reach the action animation).
	// In HL2, m_iszIdle is only used if the target NPC has SF_WAIT_FOR_SCRIPT
	// AND the SS has SF_START_ON_SPAWN.
#if 0
	const char *m_iszIdle = m_entity->ValueForKey("m_iszIdle");
	const char *m_iszEntity = m_entity->ValueForKey("m_iszEntity");
	if (m_iszIdle[0] && m_iszEntity[0])
	{
		TargetName *tn = LookupTargetNameFailOK(m_iszEntity);
		if (tn == NULL) return;

		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			// If we were looping with a trigger_relay, skip
			if (entityList[i] == m_loopEntity)
				continue;

			// If we were looping with a ourself, skip
			if (entityList[i] == m_entity)
				continue;

			// Assume an NPC and add SF_WAIT_FOR_SCRIPT
			const char *spawnflags = entityList[i]->ValueForKey("spawnflags");
			int sf = Q_atoi(spawnflags);
			sf |= 128; // wait for script
			char s[32]; sprintf(s, "%d", sf);
			entityList[i]->m_chunk->SetKeyValue("spawnflags", s);
		}
	}
#endif
}

void EC_scripted_sequence::CallBeginSequenceFromLogicAuto(void)
{
	if (!s_logicAuto)
	{
#if 1
		s_logicAuto = m_entity->AddEntity("logic_auto", NO_TARGET_NAME, "trigger_auto");
#else
		// new "entity" chunk
		char *chunkName1 = "entity";
		char *chunkName2 = new char [strlen(chunkName1) + 1];
		strcpy(chunkName2, chunkName1);
		vmf_chunk *chunk = new vmf_chunk(chunkName2);

		vmf_chunk::s_root.AddChild(chunk);

	//	num_chunks++;

		chunk->AddKeyValue("classname", "logic_auto");
//		chunk->AddKeyValue("targetname", "scripted_sequence_logic_auto"); // no targetname
		chunk->AddKeyValue("origin", m_entity->ValueForKey("origin"));

		vmf_entity *entity = new vmf_entity(chunk);

		extern vmf_entity *g_last_entity;
		g_last_entity->m_next = entity;
		g_last_entity = entity;

	//	g_num_entities++;

//		TargetName *tn = new TargetName(targetname);
//		tn->AddEntity(entity);
//		g_targetnames.Insert(targetname, tn);

		entity->m_converter = new EC_trigger_auto;
		entity->m_converter->Init(entity);
		entity->m_converter->m_converted = true;

		s_logicAuto = entity;
#endif
	}

	s_logicAuto->AddConnection(m_entity, "", "0", ONLY_ONCE_FALSE);
}

void EC_scripted_sequence::Handle_target(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;

		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			// If we were looping with a trigger_relay, don't call it
			if (entityList[i] == m_loopEntity)
				continue;

			// If we were looping with a ourself, skip
			if (entityList[i] == m_entity)
				continue;

			// If we were supposed to call another SS when this one ends
			// set the m_iszNextScript field.  It is possible to tie this
			// to an OnEndSequence output but that returns the NPC to AI.
			if (entityList[i] == m_nextSS)
			{
				AddKeyValue("m_iszNextScript", entityList[i]->UniqueName());
				continue;
			}

			m_entity->AddConnection(
				entityList[i],
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_auto )
	CONVERTED_CLASSNAME(logic_auto)
	BEST_OUTPUT(OnMapSpawn)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(triggerstate),
	XFORM_KEYVALUE(target, Handle_target), // entity to look at and track
END_CONVERTER()

// Like trigger_relay, we have on/off/toggle
void EC_trigger_auto::Handle_target(KeyValue &kv)
{
	// Add one connection for each target matching our "target" entity name.
	if (kv.m_value[0])
	{
		int triggerstate = Q_atoi( m_keyvalues->ValueForKey("triggerstate", "0") );
		if (triggerstate == 2) // Toggle
		{
			BaseClass::Handle_target(kv);
			return;
		}

		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				BestOutput(entityList[i]),
				entityList[i],
				(triggerstate == 1) ?
				entityList[i]->m_converter->OnInput(m_entity) :
				entityList[i]->m_converter->OffInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_camera )
	CONVERTED_CLASSNAME(point_viewcontrol)
	TOGGLE_INPUT(Enable)
	ON_INPUT(Enable)
	OFF_INPUT(Disable)
	BEGIN_KEYVALUES()
	XFORM_KEYVALUE(target, Handle_target), // entity to look at and track
END_CONVERTER()

void EC_trigger_camera::Handle_target(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			AddKeyValue(kv.m_key, entityList[i]->UniqueName());
			break; // should only be one
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_changelevel )
	TOGGLE_INPUT(ChangeLevel)
	ON_INPUT(ChangeLevel)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(changedelay),
	XFORM_KEYVALUE(changetarget, Handle_changetarget), // target when level changes (in the next map?)
	XFORM_KEYVALUE(landmark, Handle_landmark), // "landmark" key holds name of info_landmark
END_CONVERTER()

void EC_trigger_changelevel::Handle_changetarget(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		const char *changedelay = m_keyvalues->ValueForKey("changedelay");
		if (!changedelay[0])
			changedelay = "0";
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				entityList[i],
				"", // param
				changedelay,
				ONLY_ONCE_FALSE);
		}
	}
}

void EC_trigger_changelevel::Handle_landmark(KeyValue &kv)
{
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetName(kv.m_value);
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			if (!Q_stricmp(entityList[i]->ValueForKey("classname"), "info_landmark"))
			{
				AddKeyValue("landmark", entityList[i]->UniqueName());
				break; // should only be one
			}
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_changetarget )
	CONVERTED_CLASSNAME(ai_changetarget)
	TOGGLE_INPUT(Activate)
	BEGIN_KEYVALUES()
	KEEP_KEYVALUE(target) // entity whose "target" field we are changing
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_endsection )
	CONVERTED_CLASSNAME(point_clientcommand)
	ON_INPUT(Command)
	TOGGLE_INPUT(Command)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(section)
END_CONVERTER()

void EC_trigger_endsection::Convert( void )
{
	BaseClass::Convert();

	// Replace trigger brush with x,y,z origin
	BrushToPointEntity( m_entity );
}

bool EC_trigger_endsection::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	caller->m_chunk->AddConnection(
		output,
		m_entity->UniqueName(),
		input,
		"startupmenu force",
		delay,
		onlyOnce);

	return true;
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_hurt )
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
	RENAME_KEY(dmg, damage),
	XFORM_KEYVALUE(target, Handle_target),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_trigger_hurt::Handle_target(KeyValue &kv)
{
	if (!kv.m_value[0])
		return;

	const char *spawnflags = m_keyvalues->ValueForKey("spawnflags");
	int sf = atol(kv.m_value);

	int onlyOnce = ONLY_ONCE_FALSE;
	if (sf & 1) // Target Once
		onlyOnce = 1;

	TargetName *tn = LookupTargetNameFailOK(kv.m_value);
	if (tn == NULL) return;
	const TargetName::EntityListType &entityList = tn->GetEntityList();
	for (int i = 0; i < entityList.Count(); i++)
	{
		m_entity->AddConnection(
			"OnHurtPlayer",
			entityList[i],
			entityList[i]->m_converter->ToggleInput(m_entity),
			"", // param
			m_keyvalues->ValueForKey("delay", "0"),
			onlyOnce);

		if (!(sf & 16)) // FireClientOnly
		{
			m_entity->AddConnection(
				"OnHurt",
				entityList[i],
				entityList[i]->m_converter->ToggleInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				onlyOnce);
		}
	}
}

void EC_trigger_hurt::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = 0;
#if 0
	if (sf & 1) // Target Once
		newsf |= ; 
#endif
	if (sf & 2) // Start Off
		AddKeyValue("StartDisabled", "1"); 
	if (!(sf & 8)) // No clients
		newsf |= 1; // Applies to Clients
#if 0
	if (sf & 16) // FireClientOnly
		newsf |= ; 
#endif
	if (!(sf & 32)) // TouchClientOnly
		newsf |= 2; // Applies to NPCs
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_multiple ) // NOTE: I added "master" support to trigger_multiple/trigger_once
	BEST_OUTPUT(OnTrigger)
	MASTER_INPUT(Enable)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(delay),
	REMOVE_KEYVALUE(sounds),
	XFORM_KEYVALUE(message, Handle_message),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_trigger_multiple::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

#ifdef HUD_USEABLE
	m_useableEnt1 = NULL;
	m_useableEnt2 = NULL;

	m_message = entity->ValueForKey("message");
	if (m_message[0])
	{
		float fudge = 0;

		// If the message is OBJECT_BLOCKED_DOOR then the trigger may not
		// actually overlap the door.
		if (!Q_stricmp(m_message, "OBJECT_BLOCKED_DOOR"))
			fudge = 8;

		// This trigger_multiple is configured to display a message.
		// Try to find a useable entity within it and set the UseableString
		// of that entity.
		int numUseable = 0;
		vmf_entity *ent = NULL, *ent1 = NULL, *ent2 = NULL;
		while ((ent = FindEnclosedEntity(m_entity, ent, fudge)) != NULL)
		{
			bool useable = false;
			if (ent->IsClassname("func_door_rotating"))
				useable = true;
			else if (ent->IsClassname("func_button"))
				useable = true;
			else if (ent->IsClassname("func_letter"))
				useable = true;
			else if (ent->IsClassname("func_rot_button"))
				useable = true;
			if (useable)
			{
				if (!ent1) ent1 = ent;
				else if (!ent2) ent2 = ent;
				++numUseable;
			}
		}
		if (numUseable == 1)
		{
			m_useableEnt1 = ent1;
		}
		else if (numUseable == 2 &&
			!Q_stricmp(ent1->Classname(), ent2->Classname()) &&
			!Q_stricmp(ent1->Classname(), "func_door_rotating"))
		{
			// Double-door
			m_useableEnt1 = ent1;
			m_useableEnt2 = ent2;
		}
		else
		{
			CEMsg("*** found %d useable entities in trigger_multiple id=%s message=%s\n", numUseable, entity->ValueForKey("id"), m_message);
		}
	}
#endif
}

void EC_trigger_multiple::Finalize(void)
{
	BaseClass::Finalize();
}

bool EC_trigger_multiple::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
#ifdef HUD_USEABLE
	// If another entity wants to kill this trigger_multiple whose only purpose is
	// to display the message OBJECT_BLOCKED_DOOR then tell the
	// caller to call SetUseableString(OBJECT_DOOR) on our useable entity.
	if (!Q_stricmp(input, "Kill") &&
		m_useableEnt1 != NULL &&
		!Q_stricmp(m_message, "OBJECT_BLOCKED_DOOR"))
	{
		caller->AddConnection(output, m_useableEnt1, "SetUseableString", "#OBJECT_DOOR", "0", ONLY_ONCE_TRUE);
//		m_useableEnt1->m_callers.AddToTail(caller);

		// Double-door
		if (m_useableEnt2 != NULL)
		{
			caller->AddConnection(output, m_useableEnt2, "SetUseableString", "#OBJECT_DOOR", "0", ONLY_ONCE_TRUE);
//			m_useableEnt2->m_callers.AddToTail(caller);
		}
		return true;
	}
#endif
	return false;
}

void EC_trigger_multiple::Handle_message(KeyValue &kv)
{
	// Target a new "env_message"
	if (kv.m_value[0])
	{
#ifdef HUD_USEABLE
		if (m_useableEnt1 != NULL)
		{
			char message[128];
			if (kv.m_value[0] != '#')
				Q_snprintf(message, 128, "#%s", kv.m_value);
			else
				Q_strcpy(message, kv.m_value);
			CEMsg("trigger_multiple message=%s -> %s UseableString=%s\n", kv.m_value, m_useableEnt1->Classname(), message);
			if (!m_useableEnt1->m_chunk->HasKeyValue("UseableString", "#OBJECT_BLOCKED_DOOR"))
				m_useableEnt1->m_chunk->SetKeyValue("UseableString", message);

			// Double-door
			if (m_useableEnt2 != NULL && 
				!m_useableEnt2->m_chunk->HasKeyValue("UseableString", "#OBJECT_BLOCKED_DOOR"))
				m_useableEnt2->m_chunk->SetKeyValue("UseableString", message);

			m_entity->m_chunk->m_ignore = true;
			return;
		}
#endif
		// FIXME: add a unique "id" keyvalue to env
		vmf_entity *env = m_entity->AddEnvMessage(kv.m_value);
		m_entity->AddConnection(
			env,
			"",
			m_keyvalues->ValueForKey("delay", "0"),
			ONLY_ONCE_FALSE);
	}
}

void EC_trigger_multiple::Handle_spawnflags(KeyValue &kv)
{
	int sf = atol(kv.m_value);
	int newsf = 0;
	if (sf & 1) // monsters
		newsf |= 2; // NPCs
	if (!(sf & 2)) // no clients
		newsf |= 1; // NPCs
	if (sf & 4) // pushables
		newsf |= 4; // pushables
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_once ) // inherits from trigger_multiple above
//	BEST_OUTPUT(OnTrigger)
//	MASTER_INPUT(Enable)
	BEGIN_KEYVALUES()
//	REMOVE_KEYVALUE(delay),
//	REMOVE_KEYVALUE(sounds),
//	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_push )
	TOGGLE_INPUT(Toggle)
	BEGIN_KEYVALUES()
	RENAME_KEY(angles, pushdir),
	XFORM_KEYVALUE(speed, Handle_speed),
	XFORM_KEYVALUE(spawnflags, Handle_spawnflags),
END_CONVERTER()

void EC_trigger_push::Handle_speed(KeyValue &kv)
{
	int speed = atol(kv.m_value);

	CEMsg("dividing trigger_push speed by 2\n");
	speed /= 2;

	char s[32]; sprintf(s, "%d", speed);
	AddKeyValue(kv.m_key, s);
}

void EC_trigger_push::Handle_spawnflags(KeyValue &kv)
{
	// HL1: These don't agree with VERC listing at all
	// 1 - once only
	// 2 - start off
	int sf = atol(kv.m_value);
	int newsf = 0;
	if (sf & 1) // once only
		newsf |= 128; // once only
	if (sf & 2) // start off
		AddKeyValue("StartDisabled", "1");
	newsf |= 64; // applies to everything
	char s[32]; sprintf(s, "%d", newsf);
	AddKeyValue(kv.m_key, s);
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_relay )
	CONVERTED_CLASSNAME(logic_relay)
	OFF_INPUT(Trigger)
	ON_INPUT(Trigger)
	TOGGLE_INPUT(Trigger)
	BEST_OUTPUT(OnTrigger)
	BEGIN_KEYVALUES()
	REMOVE_KEYVALUE(triggerstate), // FIXME: possible problems with on/off/toggle thingy
END_CONVERTER()

void EC_trigger_relay::Init(vmf_entity *entity)
{
	BaseClass::Init(entity);

	const char *triggerstate = m_entity->ValueForKey("triggerstate", "0");
	m_triggerstate = Q_atoi(triggerstate);

	m_bLoopWithScriptedSequence = false;

	// If the target of this TR is a scripted_sequence, and the target of the
	// scripted_sequence is this TR, it means we are supposed to loop until the
	// trigger_relay is killed.
	const char *targetname = m_entity->ValueForKey("targetname");
	const char *target = m_entity->ValueForKey("target");
	if (targetname[0] && target[0])
	{
		TargetName *tn = LookupTargetNameFailOK(target);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
				if (!Q_stricmp(entityList[i]->Classname(), "scripted_sequence"))
				{
					const char *targetOfSS = entityList[i]->ValueForKey("target");
					if (!Q_stricmp(targetOfSS, targetname))
					{
						m_bLoopWithScriptedSequence = true;
						CEMsg("trigger_relay %s loops with scripted_sequence %s\n", targetname, target);
						break;
					}
				}
			}
		}
	}

	if (m_bLoopWithScriptedSequence)
	{
		// Don't save this entity.
		m_entity->m_chunk->m_ignore = true;
	}

#ifdef REMOVE_TRIGGER_RELAY
	// Don't save this entity.
	m_entity->m_chunk->m_ignore = true;
#endif
}

bool EC_trigger_relay::AddConnection(vmf_entity *caller, const char *output, const char *input, const char *param, const char *delay, int onlyOnce)
{
	// If this entity has already been converted, use the original keyvalues
	KeyValues &keyvalues = m_keyvalues ? *m_keyvalues : *m_entity->m_chunk->m_keyvalues;

	// Caller wants to kill this logic_relay. If we are telling a scripted_sequence to
	// keep playing itself, then tell the SS to stop looping.
	if (m_bLoopWithScriptedSequence && !Q_stricmp(input, "Kill"))
	{
		const char *target = keyvalues.ValueForKey("target");
		TargetName *tn = LookupTargetNameFailOK(target);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
				if (Q_stricmp("scripted_sequence", entityList[i]->Classname()))
					continue;

				caller->AddConnection(
					output,
					entityList[i],
					"SetLoopActionSequence", // HACK: I added this input to scripted_sequence
					"0",
					delay,
					onlyOnce);
			}
		}
		return true;
	}

#ifdef REMOVE_TRIGGER_RELAY

	const char *killtarget = keyvalues.ValueForKey("killtarget");

	// Caller wants to kill this logic_relay. If we are telling a scripted_sequence to
	// keep playing itself, then kill the scripted sequence instead.
	bool kill = !Q_stricmp(input, "Kill");
	if (kill)
	{
		if (!target[0])
			return true;
		TargetName *tn = LookupTargetNameFailOK(target);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
				if (Q_stricmp("scripted_sequence", entityList[i]->Classname()))
					continue;
	fprintf(stdout, "                                      -> %s %s\n", entityList[i]->UniqueName(), "Kill");
				caller->AddConnection(
					output,
					entityList[i],
					"Kill",
					param,
					delay,
					onlyOnce);
			}
		}
		return true;
	}

	fprintf(stdout, "EC_trigger_relay::AddConnection %s %s -> %s %s target:%s killtarget:%s\n", caller->UniqueName(), output, m_entity->UniqueName(), input, target, killtarget);

	// Tell callers to this entity to call our targets instead
	if (target[0])
	{
		TargetName *tn = LookupTargetNameFailOK(target);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
	fprintf(stdout, "                                      -> %s %s\n", entityList[i]->UniqueName(), entityList[i]->m_converter->ToggleInput(caller));
				caller->AddConnection(
					output,
					entityList[i],
					entityList[i]->m_converter->ToggleInput(caller),
					param,
					delay,
					onlyOnce);
			}
		}
	}

	// Tell callers to this entity to call our targets instead
	if (killtarget[0])
	{
		TargetName *tn = LookupTargetNameFailOK(killtarget);
		if (tn != NULL)
		{
			const TargetName::EntityListType &entityList = tn->GetEntityList();
			for (int i = 0; i < entityList.Count(); i++)
			{
	fprintf(stdout, "                                      -> %s %s\n", entityList[i]->UniqueName(), "Kill");
				caller->AddConnection(
					output,
					entityList[i],
					"Kill",
					param,
					delay,
					onlyOnce);
			}
		}
	}

	return true;
#else
	return false;
#endif
}

void EC_trigger_relay::Handle_target(KeyValue &kv)
{
	// Add one connection for each target matching our "target" entity name.
	if (kv.m_value[0])
	{
		if (m_triggerstate == 2) // Toggle
		{
			BaseClass::Handle_target(kv);
			return;
		}

		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				BestOutput(entityList[i]),
				entityList[i],
				(m_triggerstate == 1) ?
				entityList[i]->m_converter->OnInput(m_entity) :
				entityList[i]->m_converter->OffInput(m_entity),
				"", // param
				m_keyvalues->ValueForKey("delay", "0"),
				ONLY_ONCE_FALSE);
		}
	}
}

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_trigger_teleport )
	BEST_OUTPUT(OnTrigger)
	BEGIN_KEYVALUES()
	ENTITY_VALUE(target) // info_teleport_destination
END_CONVERTER()

//-----------------------------------------------------------------------------
BEGIN_CONVERTER( EC_xen_plantlight )
	BEGIN_KEYVALUES()
END_CONVERTER()

void EC_xen_plantlight::Handle_target(KeyValue &kv)
{
	// Connect OnLightOn/OnLightOff outputs to our target entities (light)
	if (kv.m_value[0])
	{
		TargetName *tn = LookupTargetNameFailOK(kv.m_value);
		if (tn == NULL) return;
		const TargetName::EntityListType &entityList = tn->GetEntityList();
		for (int i = 0; i < entityList.Count(); i++)
		{
			m_entity->AddConnection(
				"OnLightOn",
				entityList[i],
				entityList[i]->m_converter->OnInput(m_entity),
				"", // param
				"0",
				ONLY_ONCE_FALSE);

			m_entity->AddConnection(
				"OnLightOff",
				entityList[i],
				entityList[i]->m_converter->OffInput(m_entity),
				"", // param
				"0",
				ONLY_ONCE_FALSE);
		}
	}
}
