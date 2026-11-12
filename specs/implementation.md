# Vehicle Shop Statistics: behavior and implementation

Describes the retained payload **0.2.0**. The behavior prose describes the retained payload; current candidate source and metadata are identified below. Gameplay qualification is a separate requirement.


<!-- source-only-sync -->
## Current source candidate

Source version **0.2.0** is saved from workshop revision `ca32bbf79ce0840d5166a232bdb3a9fdcd761254`. The retained installable payload remains **0.2.0** with its original bytes. This source update does not rebuild or requalify that payload. The current source and metadata define the candidate; gameplay, rendered behavior and co-op remain unqualified for rebuilt bytes.

The authored behavior and lifecycle descriptions below are retained from the published payload. They are not new qualification evidence for the source candidate. The integration table below follows the current candidate metadata.
<!-- /source-only-sync -->

## Behavior

Adds a separate seven-cell specifications grid to the garage Vehicles page: base speed in km/h, storage kg, cargo slots, turn, body width, body length and heft. It reads catalog values and updates on selected-model changes. The original description stays in its own area; native status/purchase controls follow the grid. Native-style cloned sockets/text use fixed geometry rather than repeatedly replacing/autosizing the summary.

## State and lifecycle

No save or economy state is introduced. One menu owns bounded UI objects and rooted labels; original native rects are captured and restored when leaving Vehicles/closing/changing the menu. A shared LateUpdate observer reconciles action/message geometry after native setters, preserving the native one/two-button arrangement.

## Integration and configuration

No additional feature-mod runtime dependency is declared. Included shared headers are source reuse, not extra DLL dependencies.

The [module metadata](../source/mods/vehicle-shop-stats/mod.json) owns the version and declared integration contract. Native modules start at `MoonLab_ModStart`. Game/Unity integrations target the pinned Steam Windows x64 IL2CPP build.

## Implementation sources

- [mod.c](../source/mods/vehicle-shop-stats/mod.c)
- [panel.h](../source/mods/vehicle-shop-stats/panel.h)
- [base.h](../source/mods/vehicle-shop-stats/base.h)

## Verification boundary

Base stats exclude upgrades/boost and can differ from custom physics. No invented acceleration/range or measured driving-speed claim. UI resolutions, custom catalogs and rendered layout need matching game checks.

`python3 build.py --check` verifies the exported source hashes. It does not execute the mod or qualify the behaviors above. Installation and independent build prerequisites are in the [README](../README.md).
