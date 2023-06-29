#include "convert_entities.h"

bool GetBoundsOfSolid(vmf_chunk *solid, vector &min, vector &max)
{
	min.x = min.y = min.z = 1e10;
	max.x = max.y = max.z = -1e10;

	for (vmf_chunk *side = solid->LookupChild("side");
		side != 0;
		side = side->LookupNext("side"))
	{
		KeyValue *kv = side->m_keyvalues->Find("plane");
		if (kv)
		{
			vector v1, v2, v3;
			if (sscanf(kv->m_value, "(%f %f %f) (%f %f %f) (%f %f %f)",
				&v1.x, &v1.y, &v1.z,
				&v2.x, &v2.y, &v2.z,
				&v3.x, &v3.y, &v3.z) == 9)
			{
				if (v1.x < min.x) min.x = v1.x;
				if (v2.x < min.x) min.x = v2.x;
				if (v3.x < min.x) min.x = v3.x;

				if (v1.y < min.y) min.y = v1.y;
				if (v2.y < min.y) min.y = v2.y;
				if (v3.y < min.y) min.y = v3.y;

				if (v1.z < min.z) min.z = v1.z;
				if (v2.z < min.z) min.z = v2.z;
				if (v3.z < min.z) min.z = v3.z;


				if (v1.x > max.x) max.x = v1.x;
				if (v2.x > max.x) max.x = v2.x;
				if (v3.x > max.x) max.x = v3.x;

				if (v1.y > max.y) max.y = v1.y;
				if (v2.y > max.y) max.y = v2.y;
				if (v3.y > max.y) max.y = v3.y;

				if (v1.z > max.z) max.z = v1.z;
				if (v2.z > max.z) max.z = v2.z;
				if (v3.z > max.z) max.z = v3.z;
			}
		}
	}
	
	return true;
}

bool GetBounds(vmf_entity *entity, vector &min, vector &max)
{
	min.x = min.y = min.z = 1e10;
	max.x = max.y = max.z = -1e10;

	for (vmf_chunk *solid = entity->m_chunk->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		for (vmf_chunk *side = solid->LookupChild("side");
			side != 0;
			side = side->LookupNext("side"))
		{
			KeyValue *kv = side->m_keyvalues->Find("plane");
			if (kv)
			{
				vector v1, v2, v3;
				if (sscanf(kv->m_value, "(%f %f %f) (%f %f %f) (%f %f %f)",
					&v1.x, &v1.y, &v1.z,
					&v2.x, &v2.y, &v2.z,
					&v3.x, &v3.y, &v3.z) == 9)
				{
					if (v1.x < min.x) min.x = v1.x;
					if (v2.x < min.x) min.x = v2.x;
					if (v3.x < min.x) min.x = v3.x;

					if (v1.y < min.y) min.y = v1.y;
					if (v2.y < min.y) min.y = v2.y;
					if (v3.y < min.y) min.y = v3.y;

					if (v1.z < min.z) min.z = v1.z;
					if (v2.z < min.z) min.z = v2.z;
					if (v3.z < min.z) min.z = v3.z;


					if (v1.x > max.x) max.x = v1.x;
					if (v2.x > max.x) max.x = v2.x;
					if (v3.x > max.x) max.x = v3.x;

					if (v1.y > max.y) max.y = v1.y;
					if (v2.y > max.y) max.y = v2.y;
					if (v3.y > max.y) max.y = v3.y;

					if (v1.z > max.z) max.z = v1.z;
					if (v2.z > max.z) max.z = v2.z;
					if (v3.z > max.z) max.z = v3.z;
				}
			}
		}
	}

//	fprintf(stdout, "GetBounds() %f,%f %f,%f\n", min.x, min.y, max.x, max.y);
	return true;
}

bool Rotate180(vmf_entity *entity)
{
	vector min, max;

	if (!GetBounds(entity, min, max))
		return false;

	float originX, originY;
	originX = min.x + (max.x - min.x) / 2;
	originY = min.y + (max.y - min.y) / 2;

	for (vmf_chunk *solid = entity->m_chunk->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		for (vmf_chunk *side = solid->LookupChild("side");
			side != 0;
			side = side->LookupNext("side"))
		{
			KeyValue *kv = side->m_keyvalues->Find("plane");
			if (kv)
			{
				vector v1, v2, v3;
				if (sscanf(kv->m_value, "(%f %f %f) (%f %f %f) (%f %f %f)",
					&v1.x, &v1.y, &v1.z,
					&v2.x, &v2.y, &v2.z,
					&v3.x, &v3.y, &v3.z) == 9)
				{
					v1.x -= originX;
					v1.x *= -1;
					v1.x += originX;

					v1.y -= originY;
					v1.y *= -1;
					v1.y += originY;

					v2.x -= originX;
					v2.x *= -1;
					v2.x += originX;

					v2.y -= originY;
					v2.y *= -1;
					v2.y += originY;

					v3.x -= originX;
					v3.x *= -1;
					v3.x += originX;

					v3.y -= originY;
					v3.y *= -1;
					v3.y += originY;

					char plane[128];
					sprintf(plane, "(%f %f %f) (%f %f %f) (%f %f %f)",
						v1.x, v1.y, v1.z,
						v2.x, v2.y, v2.z,
						v3.x, v3.y, v3.z);

					kv->SetValue(plane);
				}
			}
		}
	}

	KeyValue *kv = entity->m_chunk->m_keyvalues->Find("origin");
	if (kv)
	{
		vector v1;

		if (sscanf(kv->m_value, "%f %f %f", &v1.x, &v1.y, &v1.z) == 3)
		{
			v1.x -= originX;
			v1.x *= -1;
			v1.x += originX;

			v1.y -= originY;
			v1.y *= -1;
			v1.y += originY;

			char origin[128];
			sprintf(origin, "%f %f %f", v1.x, v1.y, v1.z);
			kv->SetValue(origin);
		}
	}

	return true;
}

bool OriginBrushToKeyvalue(vmf_entity *entity)
{
	for (vmf_chunk *solid = entity->m_chunk->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		vmf_chunk *side = solid->LookupChild("side");
		KeyValue *kv = side->m_keyvalues->Find("material");

		if (!Q_stricmp(kv->m_value, "hl1/halflife/origin") ||
			!Q_stricmp(kv->m_value, "tools/toolsorigin") ||
			!Q_stricmp(kv->m_value, "halflife/origin"))
		{
			vector min, max;
			GetBoundsOfSolid(solid, min, max);

			vector origin;
			origin.x = min.x + (max.x - min.x) / 2;
			origin.y = min.y + (max.y - min.y) / 2;
			origin.z = min.z + (max.z - min.z) / 2;

			char szOrigin[128];
			sprintf(szOrigin, "%f %f %f", origin.x, origin.y, origin.z);
			entity->m_chunk->SetKeyValue("origin", szOrigin);

			entity->m_chunk->RemoveChild(solid);

			fprintf(stdout, "replaced ORIGIN brush with keyvalue %f,%f,%f\n", origin.x, origin.y, origin.z);

			return true;
		}
	}
	return false;
}

void BrushToPointEntity(vmf_entity *entity)
{
	for (vmf_chunk *solid = entity->m_chunk->LookupChild("solid");
		solid != 0;
		solid = solid->LookupNext("solid"))
	{
		vector min, max;
		GetBoundsOfSolid(solid, min, max);

		vector origin;
		origin.x = min.x + (max.x - min.x) / 2;
		origin.y = min.y + (max.y - min.y) / 2;
		origin.z = min.z + (max.z - min.z) / 2;

		char szOrigin[128];
		sprintf(szOrigin, "%f %f %f", origin.x, origin.y, origin.z);
		entity->m_chunk->SetKeyValue("origin", szOrigin);

		entity->m_chunk->RemoveChild(solid);

		break; // should only be one solid
	}
}

vmf_entity *FindEnclosedEntity(vmf_entity *outer, vmf_entity *first, float fudge)
{
	vector min, max;
	if (!GetBounds(outer, min, max))
		return NULL;

	min.x -= fudge, min.y -= fudge, min.z -= fudge;
	max.x += fudge, max.y += fudge, max.z += fudge;

	vmf_entity *entity = first ? first->m_next : g_entities;
	while (entity)
	{
		if (entity != outer)
		{
			vector min2, max2;
			if (GetBounds(entity, min2, max2))
			{
				if (min.x < max2.x && max.x > min2.x &&
					min.y < max2.y && max.y > min2.y &&
					min.z < max2.z && max.z > min2.z)
				{
					return entity;
				}
			}
		}
		entity = entity->m_next;
	}

	return entity;
}