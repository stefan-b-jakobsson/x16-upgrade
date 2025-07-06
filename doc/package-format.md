# Package file format standard

## File content overview

This package file format was designed to be used with the Commander X16 system
upgrade tool.

A package file consists of 

- a header and 
- one or more BLOBs.

## File header

The header is always stored at the beginning of the file, and 
it starts with the following fields, 103 bytes in total: 

| Offset | Size | Description                                    |
|--------|------|------------------------------------------------|
| $0000  | $06  | Magic string: In version 1 "X16PKG" (hex d8 31 36 d0 cb c7), in version 2 "x16pkg" (hex 58 31 36 50 4b 47) |
| $0006  | $01  | Package file format version (1 or 2)           |
| $0007  | $40  | Package description, null-terminated           |
| $0047  | $10  | Package created by, null-terminated            |
| $0057  | $0E  | Package created on (UTC): "%Y%m%d%H%M%S"       |
| $006A  | $02  | BLOB count                                     |

Then there are envelopes for the specified number of BLOBs. 
Envelopes have a fixed size of 16 bytes, as set out below:

| Offset | Size | Description                                    |
|--------|------|------------------------------------------------|
| $0000  | $01  | BLOB type                                      |
| $0001  | $03  | Version number, e.g. major-minor-patch         |
| $0004  | $03  | Size (little-endian)                           |
| $0007  | $02  | CRC-16 of the BLOB                             |
| $0009  | $07  | Reserved/unused                                |

The header finally contains a CRC-16 checksum (2 bytes) for the header itself.

The size of the header is thus 103 + 16 * number of BLOBS + 2 bytes.

All strings embedded into the header are to be encoded as PETSCII upper case/lower case.


## BLOBs

Currently, the file format supports the following BLOB types:

| ID  | Description                                              |
|-----|----------------------------------------------------------|
| $00 | Plain text                                               |
| $01 | Commander X16 Kernal ROM image                           |
| $02 | Commander X16 VERA image                                 |
| $03 | Commander X16 SMC firmware image                         |

The BLOBs are stored as binary streams in the order of their respective
header envelope. The first BLOB is stored right after the end of the header.
There is no space or delimiter between BLOBs.
