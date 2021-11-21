#include "ns.h"
#include "wh.h"

BEGIN_WH_NS


static void chasefish(PLAYER& plr, DWHActor* actor)
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
}
	
static void diefish(PLAYER& plr, DWHActor* actor)
{
	DeleteActor(actor);
}
	
static void attackfish(PLAYER& plr, DWHActor* actor)
{
	SPRITE& spr = actor->s();

	spr.z = spr.sector()->floorz;

	switch (checkfluid(actor, zr_florHit)) {
	case TYPELAVA:
		spr.hitag--;
		if (spr.hitag < 0)
			SetNewStatus(actor, DIE);
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
	
static void skirmishfish(PLAYER& plr, DWHActor* actor)
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
}
	
static void searchfish(PLAYER& plr, DWHActor* actor)
{
	aisearch(plr, actor, false);
	checksector6(actor);
}
	
static void facefish(PLAYER& plr, DWHActor* actor)
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
}

void createFishAI() {
	auto &e = enemy[FISHTYPE];
	e.info.Init(1, 1, 512, 120, 0, 32, false, 10, 0);
	e.chase = chasefish;
	e.die = diefish;
	e.attack = attackfish;
	e.skirmish = skirmishfish;
	e.search = searchfish;
	e.face = facefish;
}

void premapFish(DWHActor* actor) {
	SPRITE& spr = actor->s();

	spr.detail = FISHTYPE;
	ChangeActorStat(actor, FACE);
	enemy[FISHTYPE].info.set(spr);
}

END_WH_NS
