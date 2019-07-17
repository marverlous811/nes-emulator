# NES Emulator
- Project for learning and practies

## TODO
 - Key Milestones
  - [x] Parse iNES files
  - [x] Create Cartridges (iNES + Mapper interface)
  - [ ] CPU
    - [x] Set Up Memory Map
    - [x] Core Loop / Basic Functionality
      - Read / Write RAM
      - Addressing Modes
      - Fetch - Decode - Execute
    - [ ] Implementing Opcodes
    - [ ] Handle Interrupts
  - [ ] PPU
    - [ ] Set Up Memory Map
    - TBD
  - [ ] APU
    - TBD

 - Ongoing Tasks
  - Better error handling (something like Result in Rust)
    - [ ] Remove asserts
  - Implement more Mappers
    - [x] 000
    - [ ] 001
    - [ ] 002
    - [ ] 003
    - [ ] 004
    - [ ] 005
    - [ ] 006