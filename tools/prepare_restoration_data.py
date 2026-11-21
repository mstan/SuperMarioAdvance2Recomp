"""Prepare verified restoration data and original author notices for a build.

Input archives are maintainer inputs, kept outside Git. Release players only
need their original cartridge; no patch paths are consulted by the runtime.
"""
import argparse
import hashlib
from pathlib import Path
import zipfile


def member(archive, name, expected_size, expected_sha1):
    with zipfile.ZipFile(archive) as contents:
        if contents.namelist().count(name) != 1:
            raise ValueError(f'{archive.name}: expected one {name}')
        entry = contents.getinfo(name)
        if entry.file_size != expected_size:
            raise ValueError(f'{name}: unexpected size')
        data = contents.read(entry)
    if hashlib.sha1(data).hexdigest() != expected_sha1:
        raise ValueError(f'{name}: does not match the supported original release')
    return data


def prepare(color_archive, instrument_archive, header, notices):
    inputs = {color_archive.resolve(), instrument_archive.resolve()}
    outputs = [header, notices/'Asaki-Color-Restoration.txt',
               notices/'Weario-Instrument-Restoration.txt']
    if any(path.resolve() in inputs for path in outputs):
        raise ValueError('Output must not replace an input archive')
    payloads = [
        ('kPalettePatch', member(color_archive, 'smwcr091u.ips', 5975,
                                '984f5bd9c612e567a4b5412927b64ab65b9e2d48')),
        ('kBongoPatch', member(instrument_archive, '(US)sma2_restored_bongo.ips', 4040,
                              'b83352df126919706e3618ba4976471a32c5fc99')),
        ('kHornPatch', member(instrument_archive, '(US)sma2_restored_horn.ips', 15,
                             '96480df7633dd760dd47efe08470867fcc209482')),
    ]
    # Read every input before writing any output. Preserve the authors' text.
    with zipfile.ZipFile(color_archive) as archive:
        color_notice = archive.read('smwcr091.txt')
    with zipfile.ZipFile(instrument_archive) as archive:
        instrument_notice = archive.read('readme.txt')
    lines = ['// Generated build data; do not commit.', '#pragma once',
             '#include <array>', '#include <cstdint>',
             'namespace smw::restoration::builtin {']
    for name, data in payloads:
        lines.append(f'inline constexpr std::array<std::uint8_t, {len(data)}> {name} = {{')
        for offset in range(0, len(data), 20):
            lines.append('    ' + ','.join(f'0x{value:02x}' for value in data[offset:offset+20]) + ',')
        lines.append('};')
    lines.append('} // namespace smw::restoration::builtin')
    header.parent.mkdir(parents=True, exist_ok=True)
    notices.mkdir(parents=True, exist_ok=True)
    header.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    outputs[1].write_bytes(color_notice)
    outputs[2].write_bytes(instrument_notice)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--color-archive', type=Path, required=True)
    parser.add_argument('--instrument-archive', type=Path, required=True)
    parser.add_argument('--header', type=Path, required=True)
    parser.add_argument('--notices', type=Path, required=True)
    args = parser.parse_args()
    prepare(args.color_archive, args.instrument_archive, args.header, args.notices)
