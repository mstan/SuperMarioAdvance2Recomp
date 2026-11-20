# Optional restoration mods

The Mods catalog has two independent controls, both off by default:

- **SNES-style Colors** includes Asaki's USA Color Restoration 0.91 data.
  It changes the covered game palettes and preserves your selected display
  filter. It does not provide a complete restoration of the GBA extras.
- **Restored Bongos and Horn** includes Weario's USA Instrument Restoration 1.0
  bongo and horn data. It changes those two instruments, not the full soundtrack.

Supply the matching original USA cartridge, open **Mods**, enable the desired
control and start the game. No IPS download, file selection or configuration
edit is needed. The two choices are independent and remembered between starts.

The original cartridge file is read without modification. Native cartridge
saves remain separate. Choose these mods in the startup launcher, then start
the game. To change them after playing, quit and launch the game again; the
in-game settings menu does not expose these startup-only controls.
A launch prepares its own in-memory data image before
execution; its effective image identity is used for snapshots and diagnostic
replay. Use snapshots from the same build and feature choices. Online play
retains the existing unmodified-cartridge policy.

These controls do not enable arbitrary patched cartridges, DX or RB Select.

## Credits

Palette data: Asaki's [Color Restoration 0.91](https://www.romhacking.net/hacks/470/).
Instrument data: Weario's [Instrument Restoration 1.0](https://www.romhacking.net/hacks/4411/).
The original author readmes are included under `licenses/restoration` in the
game package. The package's MIT license covers its integration code; third-party
restoration data retains its original attribution.

## Building

Keep the original author archives outside the source tree. Configure with
`SMW_COLOR_RESTORATION_ARCHIVE` pointing to the Color Restoration 0.91 ZIP and
`SMW_INSTRUMENT_RESTORATION_ARCHIVE` pointing to Instrument Restoration 1.0's ZIP.
The Windows release helper takes the equivalent `-ColorRestorationArchive` and
`-InstrumentRestorationArchive` arguments.

The build verifies the exact supported USA payload sizes and SHA-1 identities,
then generates immutable runtime data and copies the author notices. Wrong
archives fail the build. The original ZIPs, IPS files, generated payload header,
ROM and BIOS stay outside Git. Players of the resulting package need only their
own unmodified supported cartridge and BIOS.
