cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_LIST_DIR}/tools/rp6502.cmake)

project(MegaGammaForce C CXX ASM)

# Asset tools run in the project's .venv (pip install -r requirements.txt).
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python)
    set(ART_PYTHON ${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python)
else()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(ART_PYTHON ${Python3_EXECUTABLE})
endif()

add_executable(MegaGammaForce)
rp6502_map(MegaGammaForce src/xram.h "XRAM_.*" "XRAM_IMG_.*|XRAM_SFX_DATA|XRAM_KEYBOARD|XRAM_GAMEPAD|XRAM_HUD|XRAM_BANK.*")

# Sprite and tile sheets: images/<name>.png -> <build>/<name>.bin
function(mgf_sheet name w h)
    set(out ${CMAKE_CURRENT_BINARY_DIR}/${name}.bin)
    add_custom_command(
        OUTPUT ${out}
        COMMAND ${ART_PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/tools/png2bin.py
            ${CMAKE_CURRENT_SOURCE_DIR}/images/${name}.png ${out} ${w} ${h}
        DEPENDS tools/png2bin.py images/${name}.png
        VERBATIM
    )
    string(TOUPPER ${name} up)
    rp6502_asset(MegaGammaForce XRAM(XRAM_IMG_${up}) ${out})
endfunction()

mgf_sheet(player 24 24)
mgf_sheet(flame 8 8)
mgf_sheet(vulcan 8 16)
mgf_sheet(shot8 8 8)
mgf_sheet(sword 48 32)
mgf_sheet(beam 32 64)
mgf_sheet(missile 8 8)
mgf_sheet(lock 16 16)
mgf_sheet(enemy16 16 16)
mgf_sheet(enemy32 32 32)
mgf_sheet(bullet8 8 8)
mgf_sheet(bullet16 16 16)
mgf_sheet(expl16 16 16)
mgf_sheet(expl32 32 32)
mgf_sheet(miniship 16 16)
mgf_sheet(gem 8 8)
mgf_sheet(icons 16 16)

rp6502_asset(MegaGammaForce XRAM(XRAM_PALETTES) images/palettes.bin)
rp6502_asset(MegaGammaForce XRAM(XRAM_SFX_DATA) music/sfx.bin)

# Each stage's own art, packed as ROM:stage<N>.bin and loaded into
# XRAM_BANK when the stage starts.
function(mgf_bank n)
    set(out ${CMAKE_CURRENT_BINARY_DIR}/stage${n}.bin)
    file(GLOB deps CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/stage${n}/*)
    add_custom_command(
        OUTPUT ${out}
        COMMAND ${ART_PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/tools/mkbank.py
            ${CMAKE_CURRENT_SOURCE_DIR}/images/stage${n}/bank.json ${out}
        DEPENDS tools/mkbank.py tools/png2bin.py ${deps}
        VERBATIM
    )
    rp6502_asset(MegaGammaForce stage${n}.bin ${out})
endfunction()

foreach(n 1 2 3)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/images/stage${n}/bank.json)
        mgf_bank(${n})
    endif()
endforeach()

# Music, streamed from the ROM while playing.
foreach(track Title Stage1 Boss Stage2 Stage3 Boss3 Victory)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/music/${track}.vgm)
        rp6502_asset(MegaGammaForce ${track}.vgm music/${track}.vgm)
    endif()
endforeach()
rp6502_asset(MegaGammaForce help src/help.txt)

rp6502_executable(MegaGammaForce DATA default RESET default)
rp6502_web(MegaGammaForce)

target_include_directories(MegaGammaForce PRIVATE src)
target_compile_options(MegaGammaForce PRIVATE -Os -Wall)

target_sources(MegaGammaForce PRIVATE
    src/main.c
    src/util.c
    src/gen/tables.c
    src/sprites.c
    src/bg.c
    src/hud.c
    src/input.c
    src/score.c
    src/targets.c
    src/bullets.c
    src/shots.c
    src/fx.c
    src/player.c
    src/enemies.c
    src/stage.c
    src/boss.c
    src/boss1.c
    src/boss2.c
    src/boss3.c
    src/opl.c
    src/vgm.c
    src/music.c
    src/sfx.c
)
