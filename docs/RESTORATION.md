# Optional restoration mods

The Mods catalog has two independent controls, both off by default:

- **SNES-style Colors** supports the USA Color Restoration 0.91 patch.
  It changes the covered game palettes and preserves your selected display
  filter. It does not provide a complete restoration of the GBA extras.
- **Restored Bongos and Horn** supports the USA Instrument Restoration 1.0 bongo
  and horn patches. It changes those two instruments, not the full soundtrack.

Supply the matching original USA cartridge. Obtain the external patches from
their original releases, extract the IPS files, then select them when enabling
the corresponding feature. Only the exact supported versions are accepted:

| Feature | Required file |
| --- | --- |
| SNES-style Colors | `smwcr091u.ips` from Color Restoration 0.91 |
| Restored Bongos and Horn | `(US)sma2_restored_bongo.ips` and `(US)sma2_restored_horn.ips` from Instrument Restoration 1.0 |

The cartridge and patch files are read without modification. Native cartridge
saves remain separate. Choose these mods in the startup launcher, then start
the game. To change them after playing, quit and launch the game again; the
in-game settings menu does not expose these startup-only controls.
A launch prepares its own in-memory data image before
execution; its effective image identity is used for snapshots and diagnostic
replay. Use snapshots from the same build and feature choices. Online play
retains the existing unmodified-cartridge policy.

These controls do not enable arbitrary patched cartridges, DX or RB Select.

## External patch credits

Palette data: Asaki's [Color Restoration 0.91](https://www.romhacking.net/hacks/470/).
Instrument data: Weario's [Instrument Restoration 1.0](https://www.romhacking.net/hacks/4411/).
External patch contents are not included in the source or mod packages; the
package license covers the integration code.
