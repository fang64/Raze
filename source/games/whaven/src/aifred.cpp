#include "ns.h"
#include "wh.h"

BEGIN_WH_NS

static void checkexplfred(PLAYER& plr, DWHActor* actor);


static void chasefred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;
	if (spr.lotag < 0)
		spr.lotag = 250;

	auto osectnum = spr.sectnum;
	if (cansee(plr.x, plr.y, plr.z, plr.sector, spr.x, spr.y, spr.z - (tileHeight(spr.picnum) << 7),
		spr.sectnum) && plr.invisibletime < 0) {
		if (checkdist(plr, actor)) {
			if (plr.shadowtime > 0) {
				spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047);
				SetNewStatus(actor, FLEE);
			}
			else
				SetNewStatus(actor, ATTACK);
		}
		else if (krand() % 63 > 60) {
			spr.ang = (((krand() & 128 - 256) + spr.ang + 1024) & 2047);
			SetNewStatus(actor, FLEE);
		}
		else {
			auto moveStat = aimove(actor);

			if (moveStat.type == kHitSprite) {
				if (moveStat.actor != plr.actor()) {
					int16_t daang = ((spr.ang - 256) & 2047);
					spr.ang = daang;
					if (plr.shadowtime > 0) {
						spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047);
						SetNewStatus(actor, FLEE);
					}
					else
						SetNewStatus(actor, SKIRMISH);
				}
				else {
					spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047);
					SetNewStatus(actor, SKIRMISH);
				}
			}
		}
	}
	else {
		spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047);
		SetNewStatus(actor, FLEE);
	}

	getzrange(spr.x, spr.y, spr.z - 1, spr.sectnum, (spr.clipdist) << 2, CLIPMASK0);
	if ((spr.sectnum != osectnum) && (spr.sector()->lotag == 10))
		warpsprite(actor);

	if (checksector6(actor))
		return;

	processfluid(actor, zr_florHit, false);

	if (sector[osectnum].lotag == KILLSECTOR) {
		spr.hitag--;
		if (spr.hitag < 0)
			SetNewStatus(actor, DIE);
	}

	SetActorPos(actor, &spr.pos);

	if (zr_florHit.type == kHitSector && (spr.sector()->floorpicnum == LAVA
		|| spr.sector()->floorpicnum == LAVA1 || spr.sector()->floorpicnum == ANILAVA)) {
		spr.hitag--;
		if (spr.hitag < 0)
			SetNewStatus(actor, DIE);
	}

	checkexplfred(plr, actor);
}
	
static void skirmishfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;

	if (spr.lotag < 0)
		SetNewStatus(actor, FACE);
	auto osectnum = spr.sectnum;
	if (aimove(actor).type != kHitNone) {
		spr.ang = getangle(plr.x - spr.x, plr.y - spr.y);
		SetNewStatus(actor, FACE);
	}
	if ((spr.sectnum != osectnum) && (spr.sector()->lotag == 10))
		warpsprite(actor);

	processfluid(actor, zr_florHit, false);

	SetActorPos(actor, &spr.pos);

	if (checksector6(actor))
		return;

	checkexplfred(plr, actor);
}
	
static void diefred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;

	if (spr.lotag <= 0) {
		spr.picnum++;
		spr.lotag = 20;

		if (spr.picnum == FREDDEAD) {
			if (difficulty == 4)
				SetNewStatus(actor, RESURECT);
			else {
				kills++;
				SetNewStatus(actor, DEAD);
			}
		}
	}
}
	
static void searchfred(PLAYER& plr, DWHActor* actor)
{
	aisearch(plr, actor, false);
	if (!checksector6(actor))
		checkexplfred(plr, actor);
}
	
static void frozenfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;
	if (spr.lotag < 0) {
		spr.pal = 0;
		spr.picnum = FRED;
		SetNewStatus(actor, FACE);
	}
}
	
static void facefred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	boolean cansee = ::cansee(plr.x, plr.y, plr.z, plr.sector, spr.x, spr.y, spr.z - (tileHeight(spr.picnum) << 7),
		spr.sectnum);

	if (cansee && plr.invisibletime < 0) {
		spr.ang = (getangle(plr.x - spr.x, plr.y - spr.y) & 2047);

		if (plr.shadowtime > 0) {
			spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047);
			SetNewStatus(actor, FLEE);
		}
		else {
			actor->SetOwner(plr.actor());
			SetNewStatus(actor, CHASE);
		}
	}
	else { // get off the wall
		if (actor->GetOwner() == plr.actor()) {
			spr.ang = (((krand() & 512 - 256) + spr.ang) & 2047);
			SetNewStatus(actor, FINDME);
		}
		else if (cansee) SetNewStatus(actor, FLEE);
	}

	if (checkdist(plr, actor))
		SetNewStatus(actor, ATTACK);

	checkexplfred(plr, actor);
}
	
static void attackfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	getzrange(spr.x, spr.y, spr.z - 1, spr.sectnum, (spr.clipdist) << 2, CLIPMASK0);
	spr.z = zr_florz;

	switch (checkfluid(actor, zr_florHit)) {
	case TYPELAVA:
		spr.hitag--;
		if (spr.hitag < 0)
			SetNewStatus(actor, DIE);
	case TYPEWATER:
		spr.z += tileHeight(spr.picnum) << 5;
		break;
	}

	SetActorPos(actor, &spr.pos);

	if (spr.lotag >= 64) {
		if (checksight(plr, actor))
			if (checkdist(plr, actor)) {
				spr.ang = checksight_ang;
				attack(plr, actor);
			}
	}
	else if (spr.lotag < 0) {
		if (plr.shadowtime > 0) {
			spr.ang = (((krand() & 512 - 256) + spr.ang + 1024) & 2047); // NEW
			SetNewStatus(actor, FLEE);
		}
		else
			SetNewStatus(actor, CHASE);
	}
	spr.lotag -= TICSPERFRAME;

	checksector6(actor);
}
	
static void fleefred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;
	auto osectnum = spr.sectnum;

	auto moveStat = aimove(actor);

	if (moveStat.type != kHitNone) {
		if (moveStat.type == kHitWall) {
			int nWall = moveStat.index;
			int nx = -(wall[wall[nWall].point2].y - wall[nWall].y) >> 4;
			int ny = (wall[wall[nWall].point2].x - wall[nWall].x) >> 4;
			spr.ang = getangle(nx, ny);
		}
		else {
			spr.ang = getangle(plr.x - spr.x, plr.y - spr.y);
			SetNewStatus(actor, FACE);
		}
	}
	if (spr.lotag < 0)
		SetNewStatus(actor, FACE);

	if ((spr.sectnum != osectnum) && (spr.sector()->lotag == 10))
		warpsprite(actor);

	if (checksector6(actor))
		return;

	processfluid(actor, zr_florHit, false);

	SetActorPos(actor, &spr.pos);

	checkexplfred(plr, actor);
}
	
static void painfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;
	if (spr.lotag < 0) {
		spr.picnum = FRED;
		spr.ang = plr.angle.ang.asbuild();
		SetNewStatus(actor, FLEE);
	}

	aimove(actor);
	processfluid(actor, zr_florHit, false);
	SetActorPos(actor, &spr.pos);

	checkexplfred(plr, actor);
}
	
static void resurectfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.lotag -= TICSPERFRAME;
	if (spr.lotag < 0) {
		SetNewStatus(actor, FACE);
		spr.picnum = FRED;
		spr.hitag = adjusthp(40);
		spr.lotag = 100;
		spr.cstat |= 1;
	}
}

static void checkexplfred(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	WHSectIterator it(spr.sectnum);
	while (auto sectactor = it.Next())
	{
		SPRITE& tspr = sectactor->s();

		int dx = abs(spr.x - tspr.x); // x distance to sprite
		int dy = abs(spr.y - tspr.y); // y distance to sprite
		int dz = abs((spr.z >> 8) - (tspr.z >> 8)); // z distance to sprite
		int dh = tileHeight(tspr.picnum) >> 1; // height of sprite
		if (dx + dy < PICKDISTANCE && dz - dh <= getPickHeight()) {
			if (tspr.picnum == EXPLO2
				|| tspr.picnum == SMOKEFX
				|| tspr.picnum == MONSTERBALL) {
				spr.hitag -= TICSPERFRAME << 2;
				if (spr.hitag < 0) {
					SetNewStatus(actor, DIE);
				}
			}
		}
	}
}

void createFredAI() {
	auto &e = enemy[FREDTYPE];
	e.info.Init(48, 48, 1024 + 256, 120, 0, 64, false, 40, 0);
	e.chase = chasefred;
	e.skirmish = skirmishfred;
	e.die = diefred;
	e.search = searchfred;
	e.frozen = frozenfred;
	e.face = facefred;
	e.attack = attackfred;
	e.flee = fleefred;
	e.pain = painfred;
	e.resurect = resurectfred;
}


void premapFred(DWHActor* actor) {
	SPRITE& spr = actor->s();
	if (spr.picnum == FRED && spr.pal == 1) {
		DeleteActor(actor);
		return;
	}
	spr.detail = FREDTYPE;
	ChangeActorStat(actor, FACE);
	enemy[FREDTYPE].info.set(spr);

	if (krand() % 100 > 50)
		spr.extra = 1;
}

END_WH_NS
