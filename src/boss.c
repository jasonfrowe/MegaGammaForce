#include "boss.h"
#include "bullets.h"
#include "fx.h"
#include "game.h"
#include "hud.h"
#include "player.h"
#include "score.h"
#include "sfx.h"
#include "sprites.h"
#include "targets.h"
#include "util.h"

enum
{
    BS_OFF,
    BS_ENTER,
    BS_FIGHT,
    BS_DYING,
    BS_ESCAPE,
    BS_DONE,
};

uint16_t boss_t;
int16_t boss_x, boss_y;
int16_t part_px[BOSS_PARTS_MAX], part_py[BOSS_PARTS_MAX];
uint16_t part_hp[BOSS_PARTS_MAX];
bool part_dead[BOSS_PARTS_MAX];
uint8_t part_aim[BOSS_PARTS_MAX];
uint8_t boss_destruction;
bool boss_escaped;

static const boss_def_t *def;
static uint8_t state;
static uint16_t state_t;
static uint8_t flash[BOSS_PARTS_MAX];
static uint8_t shown_pct = 0xFF, shown_hp = 0xFF;

static void hide_all(void)
{
    for (uint8_t i = 0; i < SPR_BOSS_N; i++)
        spr_hide(&spr[SPR_BOSS + i]);
}

static void finish(void)
{
    hide_all();
    if (def && def->finish)
        def->finish();
}

void boss_reset(void)
{
    spr_hide(&hspr[HSPR_PORTRAIT]);
    if (state != BS_OFF && state != BS_DONE)
        finish();
    state = BS_OFF;
    def = 0;
    hide_all();
    shown_pct = shown_hp = 0xFF;
}

void boss_start(const boss_def_t *d)
{
    def = d;
    state = BS_ENTER;
    boss_t = state_t = 0;
    boss_x = Q4(PF_W / 2);
    boss_y = Q4(-60);
    boss_destruction = 0;
    boss_escaped = false;
    for (uint8_t i = 0; i < def->n; i++) {
        const part_t *p = &def->parts[i];
        part_hp[i] = p->hp;
        flash[i] = 0;
        part_dead[i] = false;
        part_aim[i] = 0;
        if (p->kind != PK_HIDDEN)
            spr_setup(&spr[SPR_BOSS + i], p->img, PAL_ADDR(PAL_BOSS), WH(p->w, p->h), OPT4 | p->opt);
    }
    if (def->start)
        def->start();
    shown_pct = shown_hp = 0xFF;
}

bool boss_done(void)
{
    return state == BS_DONE;
}

bool boss_fighting(void)
{
    return state == BS_FIGHT;
}

uint8_t boss_aim(uint8_t i)
{
    uint8_t a = angle_to((pl_x >> 4) - part_px[i], (pl_y >> 4) - part_py[i]);
    part_aim[i] = (uint8_t)(((uint8_t)(a - ANG_DOWN + 16)) >> 5);
    return a;
}

static void destroy_part(uint8_t i, bool scored)
{
    if (part_dead[i])
        return;
    const part_t *p = &def->parts[i];
    part_dead[i] = true;
    part_hp[i] = 0;
    int16_t x = part_px[i], y = part_py[i];
    fx_explode(x, y, p->w >= 32);
    if (p->h >= 64) {
        fx_explode(x, y - 20, 1);
        fx_explode(x, y + 20, 0);
    }
    sfx_enemy(SFX_BIG_EXPLODE, SFX_PRI_EVENT);
    if (scored) {
        boss_destruction += p->weight;
        score_add(p->score);
        weapon_xp((uint8_t)(2 + (p->weight >> 1)));
    }
    if (def->destroyed)
        def->destroyed(i);
    /* Parts mounted on this one go with it. */
    for (uint8_t k = 0; k < def->n; k++)
        if (def->parts[k].parent == i)
            destroy_part(k, scored);
}

void boss_damage(uint8_t i, uint8_t dmg)
{
    if (state != BS_FIGHT || i >= def->n || part_dead[i])
        return;
    flash[i] = 3;
    if ((boss_t & 7) == 0)
        sfx_enemy(SFX_BOSS_HIT, SFX_PRI_FIRE);
    score_add((uint16_t)dmg * 20); /* every hit on the boss scores */
    if (part_hp[i] <= dmg) {
        destroy_part(i, true);
        if (i == def->core) {
            state = BS_DYING;
            state_t = 0;
            bullets_cancel_all();
        }
    } else {
        part_hp[i] -= dmg;
    }
}

static void place(void)
{
    int16_t x = boss_x >> 4, y = boss_y >> 4;
    for (uint8_t i = 0; i < def->n; i++) {
        part_px[i] = x + def->parts[i].ox;
        part_py[i] = y + def->parts[i].oy;
    }
    if (def->place)
        def->place();
}

static void draw(void)
{
    for (uint8_t i = 0; i < def->n; i++) {
        const part_t *p = &def->parts[i];
        if (p->kind == PK_HIDDEN)
            continue;
        mode5_csprite_t *s = &spr[SPR_BOSS + i];
        if (part_dead[i] && p->kind == PK_TURRET) {
            spr_hide(s);
            continue;
        }
        s->x_pos_px = PF_X + part_px[i] - (p->w >> 1);
        s->y_pos_px = part_py[i] - (p->h >> 1);
        uint8_t pal = PAL_BOSS;
        if (part_dead[i])
            pal = PAL_BOSS_WRECK;
        else if (flash[i]) {
            /* Big parts flash on alternate frames; full white is too much. */
            if (--flash[i] & 1)
                pal = PAL_FLASH;
        }
        s->palette_ptr = PAL_ADDR(pal);
        if (p->kind == PK_TURRET)
            s->xram_sprite_ptr = p->img + part_aim[i] * (uint16_t)(p->w * p->h / 2);
        if (def->frame)
            def->frame(i, s);
    }
}

void boss_update(void)
{
    if (state == BS_OFF || state == BS_DONE)
        return;
    boss_t++;
    state_t++;

    switch (state) {
    case BS_ENTER:
        if (def->enter()) {
            state = BS_FIGHT;
            state_t = 0;
        }
        break;
    case BS_FIGHT:
        def->fight();
        if (state_t > def->time_limit) {
            state = BS_ESCAPE;
            boss_escaped = true;
            bullets_cancel_all();
        }
        break;
    case BS_DYING:
        /* Chain of explosions across the whole boss, then the big one. */
        if ((state_t & 5) == 0) {
            uint8_t i = (uint8_t)(rng8() % def->n);
            fx_explode(part_px[i] + (int8_t)((rng8() & 31) - 16), part_py[i] + (int8_t)((rng8() & 31) - 16),
                       rng8() & 1);
            if ((state_t & 15) == 0)
                sfx_enemy(SFX_BIG_EXPLODE, SFX_PRI_TOP);
        }
        boss_y += 4;
        if (def->dying)
            def->dying();
        if (state_t > 150) {
            for (uint8_t i = 0; i < def->n; i++)
                destroy_part(i, false);
            finish();
            state = BS_DONE;
            return;
        }
        break;
    case BS_ESCAPE:
        boss_y -= 24;
        if (boss_y < Q4(-240)) {
            finish();
            state = BS_DONE;
            return;
        }
        break;
    }

    place();
    if (state == BS_FIGHT) {
        for (uint8_t i = 0; i < def->n; i++) {
            if (!part_dead[i])
                tgt_add(part_px[i], part_py[i], def->parts[i].hw, def->parts[i].hh, (uint8_t)(TGT_BOSS + i));
        }
    }
    draw();
}

void boss_hud(bool force)
{
    if (state == BS_OFF) {
        return;
    }
    /* Right panel, below the lives: the boss's portrait and name, how much
     * of it is destroyed, and its core's health. */
    if (force || shown_pct == 0xFF) {
        mode5_csprite_t *s = &hspr[HSPR_PORTRAIT];
        spr_setup(s, def->portrait, PAL_ADDR(PAL_BOSS), WH(16, 16), OPT4);
        s->x_pos_px = 33 * 8;
        s->y_pos_px = 14 * 8;
        hud_text(36, 14, "BOSS", HC_RED, HC_PANEL);
        hud_text(33, 16, "DESTROY", HC_GRAY, HC_PANEL);
        hud_text(33, 19, def->core_label, HC_GRAY, HC_PANEL);
    }
    if (boss_destruction != shown_pct || force) {
        shown_pct = boss_destruction;
        hud_u16(33, 17, boss_destruction, 3, boss_destruction >= 100 ? HC_YELLOW : HC_WHITE, HC_PANEL);
        hud_text(36, 17, "%", HC_GRAY, HC_PANEL);
    }
    uint8_t c = def->core;
    uint8_t h = (uint8_t)(((uint32_t)part_hp[c] * 14) / def->parts[c].hp);
    if (h != shown_hp || force) {
        shown_hp = h;
        hud_bar(33, 20, 7, h, HC_RED);
    }
}
