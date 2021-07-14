package main

import (
	"fmt"
	"io/ioutil"
// 	"random"
	"regexp"
// 	"shutil"
// 	"struct"
// 	"unicodedata"
    "bits"
)

//#######################################################################
// 68000 interpreter: minimal user-mode implementation
//
// Notes {
// - all "sizes" are in bytes
// - mem is a bytearray, which must be populated from outside the block
// - minimise direct access to the mem bytearray, preferring read()/write()
// - define lineA(), lineF(), check_for_lurkers() somewhere outside this block
//#######################################################################

// Emulator state: set to something better than this before running
var pc uint32
const regs = 0x80000
var x, n, z, v, c bool
var mem []byte

func read(numbytes int, addr uint32) (val uint32) {
    for i := range numbytes {
        val <<= 8
        val += mem[addr+i]
    }
}

func write(numbytes int, addr uint32, val uint32) {
    for i := range numbytes {
        mem[addr+i] = byte(val)
        val >>= 8
    }
}

func push(size int, data uint32) {
    ptr := address_by_mode(39, size) // -(A7)
    write(size, ptr, data)
}

func pop(size int) uint32 {
    ptr := address_by_mode(31, size) // (A7)+
    return read(size, ptr)
}

func get_sp() uint32 {
    return read(4, regs + 60)
}

func set_sp(val uint32) {
    write(4, regs + 60, val)
}

func signed(size int, n uint32) int32 {
    shift := 32 - (8 * size)
    return (int32(n) << shift) >> shift
}

func add_then_set_vc(a uint32, b uint32, size int) (result uint32) {
    result = a + b
    signbit = 1 << ((size * 8) - 1)
    v = (a & signbit) == (b & signbit) != (result & signbit)
    c = result < a // could use result < b just as easily
}

func sub_then_set_vc(a uint32, b uint32, size int) { // subtract b from a
    result = a - b
    signbit = 1 << ((size * 8) - 1)
    v = (a & signbit) == (b & signbit) != (result & signbit)
    c = a < b
}

func set_nz(num uint32, size int) {
//    global n, z
    bitsize = size * 8
    n = bool(num & (1 << (bitsize - 1)))
    z = num == 0
}

func get_ccr() (ccr uint32) {
    for shift, value := range []bool{c, v, z, n, x} {
        if value {
            ccr |= (1 << shift)
        }
    }
}

func set_ccr(to_byte) {
    x = to_byte & 16 != 0
    n = to_byte & 8 != 0
    z = to_byte & 4 != 0
    v = to_byte & 2 != 0
    c = to_byte & 1 != 0
}

func address_by_mode(mode, size) (ptr uint32) { // mode given by bottom 6 bits
    // side effects: predecrement/postincrement, advance pc to get extension word

    if mode < 16 { // Dn or An -- optimise common case slightly
        ptr = regs + (mode & 15) * 4 + 4 - size
    } else if mode >> 3 == 2 { // (An)
        ptr = read(4, regs + 32 + (mode & 7) * 4)
    } else if mode >> 3 == 3 { // (An)+
        regptr = regs + 32 + (mode & 7) * 4
        ptr = read(4, regptr)
        newptr = ptr + size
        if mode & 7 == 7 && size == 1 {
            newptr += 1
        }
        write(4, regptr, newptr)
    } else if mode >> 3 == 4 { // -(An)
        regptr = regs + 32 + (mode & 7) * 4
        ptr = read(4, regptr)
        ptr -= size
        if mode & 7 == 7 && size == 1 {
            ptr--
        }
        write(4, regptr, ptr)
    } else if mode >> 3 == 5 { // d16(An)
        regptr = regs + 32 + (mode & 7) * 4
        ptr = read(4, regptr) + signed(2, read(2, pc)); pc += 2
    } else if mode >> 3 == 6 { // d8(An,Xn)
        ptr = regs + 32 + (mode & 7) * 4
        ptr = read(4, ptr) // get An

        xreg = read(1, pc); pc++
        xofs = signed(1, read(1, pc)); pc++
        whichreg = regs + (xreg >> 4) * 4 // could be D or A
        if xreg & 8 {
            rofs = signed(4, read(4, whichreg))
        } else {
            rofs = signed(2, read(2, whichreg + 2))
        }
        ptr += xofs + rofs
    } else if mode == 58 { // d16(PC)
        ptr = pc + signed(2, read(2, pc)); pc += 2
    } else if mode == 59 { // d8(PC,Xn)
        ptr = pc
        xreg = read(1, pc); pc++
        xofs = signed(1, read(1, pc)); pc++
        whichreg = regs + (xreg >> 4) * 4 // could be D or A
        if xreg & 8 {
            rofs = signed(4, read(4, whichreg))
        } else {
            rofs = signed(2, read(2, whichreg + 2))
        }
        ptr += xofs + rofs
    } else if mode == 56 { // abs.W
        ptr = read(2, pc); pc += 2
    } else if mode == 57 { // abs.L
        ptr = read(4, pc); pc += 4
    } else if mode == 60 { // #imm
        if size == 1 {
            ptr = pc + 1; pc += 2
        } else if size == 2 {
            ptr = pc; pc += 2
        } else if size == 4 {
            ptr = pc; pc += 4
        }
    } else {
        panic("reserved addressing mode")
    }
}

func test_condition(int cond) bool {
    if cond == 0 { // T true
        return true
    } else if cond == 1 { // F false
        return false
    } else if cond == 2 { // HI higher than
        return !(c || z)
    } else if cond == 3 { // LS lower or same
        return c || z
    } else if cond == 4 { // CC carry clear aka HS
        return !c
    } else if cond == 5 { // CS carry set aka LS
        return c
    } else if cond == 6 { // NE not equal
        return !z
    } else if cond == 7 { // EQ equal
        return z
    } else if cond == 8 { // VC overflow clear
        return !v
    } else if cond == 9 { // VS overflow set
        return v
    } else if cond == 10 { // PL plus
        return !n
    } else if cond == 11 { // MI minus
        return n
    } else if cond == 12 { // GE greater or equal
        return n == v
    } else if cond == 13 { // LT less than
        return !(n == v)
    } else if cond == 14 { // GT greater than
        return n == v && !z
    } else if cond == 15 { // LE less or equal
        return n != v || z
    }
}

func line0(inst uint16) {
    if inst & 256 || inst >> 9 == 4 { // btst,bchg,bclr,bset (or movep)
        if inst & 256 { // bit numbered by data register
            if (inst >> 3) & 7 == 1 {
                panic("movep")
            }
            dn = inst >> 9
            bit := read(4, regs + dn * 4)
        } else { // bit numbered by immediate
            bit = read(2, pc); pc += 2
        }

        mode = inst & 63
        if mode >> 3 <= 1 {
            size = 4 // applies to register
        } else {
            size = 1 // applies to memory address
        }
        bit %= size * 8
        mask = 1 << bit

        ptr = address_by_mode(mode, size)
        val = read(size, ptr)
        z = !(val & mask)

        if (inst >> 6) & 3 == 1 { // bchg
            val ^= mask
        } else if (inst >> 6) & 3 == 2 { // bclr
            val &= ^mask
        } else if (inst >> 6) & 3 == 3 { // bset
            val |= mask
        }
        // ^^ the btst case is already handled by setting z

        write(size, ptr, val)

    } else { //ori,andi,subi,addi,eori -- including to SR/CCR
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {
            raise("ori/andi/subi/addi/eori size=%11")
        }

        src_ptr = address_by_mode(60, size) // '#imm' mode, advances pc
        imm = read(size, src_ptr)

        // now, are we operating on a special mode?
        dest_mode = inst & 63
        if dest_mode == 60 { // '#imm' actually means CCR/SR
            // for addi/subi this would simply be invalid
            val = get_ccr()
        } else {
            dest_ptr = address_by_mode(dest_mode, size)
            val = read(size, dest_ptr)

        }
        if inst >> 9 == 0 { // ori
            val |= imm
            v = false
            c = false
            set_nz(val, size)
        } else if inst >> 9 == 1 { // andi
            val &= imm
            v = false
            c = false
            set_nz(val, size)
        } else if inst >> 9 == 2 { // subi
            val = sub_then_set_vc(val, imm, size)
            x = c
            set_nz(val, size)
        } else if inst >> 9 == 3 { // addi
            val = add_then_set_vc(val, imm, size)
            x = c
            set_nz(val, size)
        } else if inst >> 9 == 5 { // eori
            val ^= imm
            v = false
            c = false
            set_nz(val, size)
        } else if inst >> 9 == 6 { // cmpi: same as subi, but don't set
            fake_val = sub_then_set_vc(val, imm, size)
            set_nz(fake_val, size)

        }
        if dest_mode == 60 {
            set_ccr(val)
        } else {
            write(size, dest_ptr, val)
        }
    }
}

func line123(inst uint16) { // move, and movea which is a special-ish case
//    global v, c
    size = []int{0, 1, 4, 2}[(inst >> 12) & 3]
    if size == 0 {panic("move size=%00")
    }
    dest_mode = ((inst >> 3) & 0x38) | ((inst >> 9) & 7)
    src_mode = inst & 63

    src := address_by_mode(src_mode, size)
    datum = signed(size, read(size, src))

    if dest_mode >> 3 == 1 { // movea: sign extend to 32 bits
        size = 4
    } else { // non-movea: set condition codes
        v = false
        c = false
        set_nz(datum, size)

    }
    dest = address_by_mode(dest_mode, size)
    write(size, dest, datum)
}

func line4(inst uint16) { // very,crowded,line
//    global pc, x, n, z, v, c

    if (inst >> 6) & 63 == 3 { // move from sr
        dest = address_by_mode(inst & 63, 2) // sr is 2 bytes
        write(2, dest, get_ccr())
    } else if (inst >> 6) & 0x3f == 0x37 { // move to sr/ccr
        if (inst >> 6) & 8 != 0 {
            size = 1 // ccr
        } else {
            size = 2 // sr
        }
        src = address_by_mode(inst & 63, size)
        set_ccr(read(size, src))
    } else if (inst >> 8) & 15 == 0 || (inst >> 8) & 15 == 4 || (inst >> 8) & 15 == 6 { // negx,neg,not
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("negx/neg/not size=%11")
        }
        dest = address_by_mode(inst & 63, size)
        datum = read(size, dest)

        if (inst >> 8) & 15 == 0 { // negx
            datum = sub_then_set_vc(0, datum+x, size)
            x = c
            set_nz(datum, size)
        } else if (inst >> 8) & 15 == 4 { // neg
            datum = sub_then_set_vc(0, datum, size)
            x = c
            set_nz(datum, size)
        } else { // not
            datum = ^datum
            v = false
            c = false
            set_nz(datum, size)
        }
        write(size, dest, datum)

    } else if (inst >> 8) & 15 == 2 { // clr
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("clr size=%11")
        }
        dest = address_by_mode(inst & 63, size)

        n = false
        z = true
        v = false
        c = false

        write(size, dest, 0)
    } else if inst & 0xFB8 == 0x880 { // ext
        if inst & 64 != 0 {
            size = 4
        } else {
            size = 2
        }
        dn = inst & 7
        src = regs + dn * 4 + 4 - size/2
        dest = regs + dn * 4 + 4 - size
        datum = signed(size/2, read(size/2, src))
        set_nz(datum, size)
        v = false
        c = false
        write(size, dest, datum)
    } else if inst & 0xFF8 == 0x840 { // swap.w
        dn = inst & 7
        dest = regs + dn * 4
        datum = read(4, dest)
        datum = ((datum >> 16) & 0xFFFF) | ((datum & 0xFFFF) << 16)
        set_nz(datum, 4)
        v = false
        c = false
        write(4, dest, datum)
    } else if inst & 0xFC0 == 0x840 { // pea -- notice similarity to swap.w
        ea = address_by_mode(inst & 63, 4) // size doesn't matter here
        push(4, ea)
    } else if inst & 0xF00 == 0xA00 { // tst,tas
        size = []int{1, 2, 4, 1}[(inst >> 6) & 3]
        is_tas = (inst >> 6) & 3 == 3
        dest = address_by_mode(inst & 63, size)
        datum = read(size, dest)
        set_nz(datum, size)
        v = false
        c = false
        if is_tas {
            write(1, dest, datum | 0x80)
        }
    } else if inst & 0xFF8 == 0xE50 { // link
        an = inst & 7; an_ea = regs + 32 + an * 4
        imm = signed(2, read(2, pc)); pc += 2

        push(4, read(4, an_ea)) // move.l a6,-(sp)
        sp = read(4, regs+60)
        write(4, an_ea, sp) // move.l sp,a6
        write(4, regs+60, sp + imm) // add.w #imm,sp

    } else if inst & 0xFF8 == 0xE58 { // unlk
        an = inst & 7; an_ea = regs + 32 + an * 4

        write(4, regs+60, read(4, an_ea)) // move.l a6,sp
        write(4, an_ea, pop(4)) // move.l (sp)+,a6
    } else if inst & 0xFFF == 0xE71 { // nop
    } else if inst & 0xFFF == 0xE75 { // rts
        check_for_lurkers()
        pc = pop(4)
    } else if inst & 0xFFF == 0xE77 { // rtr
        set_ccr(pop(2))
        pc = pop(4)
    } else if inst & 0xF80 == 0xE80 { // jsr/jmp
        targ = address_by_mode(inst & 63, 4) // any size
        if !inst & 0x40 {
            check_for_lurkers() // don't slow down jmp's, we do them a lot
            push(4, pc)
        }
        pc = targ
    } else if inst & 0xF80 == 0x880 { // movem registers,ea
        size := 2
        if (inst & 64 != 0) {
            size = 4
        }

        which := read(2, pc); pc += 2
        totalsize := bits.OnesCount16(which) * size

        mode := inst & 63
        ptr := address_by_mode(mode, totalsize)

        if mode >> 3 == 4 { // reverse the bits if predecrementing
            which = bits.Reverse16(which)
        }

        for reg := 0; reg < 16; reg++ {
            if which & (1 << reg) != 0 {
                regptr = regs + reg * 4 + 4 - size
                write(size, ptr, read(size, regptr))
                ptr += size
            }
        }
    } else if inst & 0xF80 == 0xC80 { // movem ea,registers
        size := 2
        if (inst & 64 != 0) {
            size = 4
        }

        which := read(2, pc); pc += 2
        totalsize := bits.OnesCount16(which) * size

        mode := inst & 63
        ptr := address_by_mode(mode, totalsize)

        for reg := 0; reg < 16; reg++ {
            if which & (1 << reg) != 0 {
                if !(reg >= 8 && mode >> 3 == 3 && reg & 7 == mode & 7) {
                    regptr := regs + reg * 4 + 4 - size
                    write(4, regptr, signed(size, read(size, ptr)))
                }
                ptr += size
            }
        }
    } else if inst & 0x1C0 == 0x1C0 { // lea
        an = (inst >> 9) & 7
        ea = address_by_mode(inst & 63, 4) // any size
        write(4, regs + 32 + an * 4, ea)
    } else if inst & 0x1C0 == 0x180 { // chk
        dn = (inst >> 9) & 7
        testee = signed(2, read(2, regs + 4 * dn + 2))
        ea = address_by_mode(inst & 63, 2)
        ubound = signed(2, read(2, ea))
        if !0 <= testee <= ubound: raise ValueError("chk failed") // raise exception
        }
    }
}

func line5(inst uint16) { // addq,subq,scc,dbcc
//    global pc, x, n, z, v, c
    if (inst >> 6) & 3 != 3 { // addq,subq
        size = 1 << ((inst >> 6) & 3)
        if size == 2 && (inst >> 3) & 7 == 1: size = 4 // An.w is really An.l

        }
        imm = ((inst >> 9) & 7) || 8

        dest = address_by_mode(inst & 63, size)
        datum = read(size, dest)
        save_ccr = x, n, z, v, c
        if inst & 256 { // subq
            datum := sub_then_set_vc(datum, imm, size)
        } else { // addq
            datum = add_then_set_vc(datum, imm, size)
        }
        x = c
        set_nz(datum, size)
        if (inst >> 3) & 7 == 1: x, n, z, v, c = save_ccr // touch not ccr if dest is An
        }
        write(size, dest, datum)

    } else if (inst >> 3) & 7 == 1 { // dbcc
        check_for_lurkers()

        disp = signed(2, read(2, pc)) - 2; pc += 2

        cond = (inst >> 8) & 15 // if cond satisfied then DO NOT take loop
        if test_condition(cond): return

        }
        // decrement the counter (dn)
        dn = inst & 7
        dest = regs + dn * 4 + 2
        counter = (read(2, dest) - 1) & 0xFFFF
        write(2, dest, counter)
        if counter == 0xFFFF: return // do not take the branch

        }
        pc += disp

    } else { // scc
        dest = address_by_mode(inst & 63, 1)
        cond = (inst >> 8) & 15
        write(1, dest, 0xFF * test_condition(cond))
    }
}

func line6(inst uint16) { // bra,bsr,bcc
//    global pc

    check_for_lurkers()

    disp = signed(1, inst & 255)
    if disp == 0 { // word displacement
        disp = signed(2, read(2, pc)) - 2; pc += 2

    }
    cond = (inst >> 8) & 15
    if cond > 1 && !test_condition(cond): return // not taken

    }
    if cond == 1 { // is bsr
        push(4, pc)

    }
    pc += disp
}

func line7(inst uint16) { // moveq
//    global n, z, v, c
    dn = (inst >> 9) & 7
    val = signed(1, inst & 255)
    n = (val < 0); z = (val == 0); v = c = 0
    write(4, regs + dn * 4, val)
}

func line8(inst uint16) { // divu,divs,sbcd,or
//    global n, z, v, c
    if inst & 0x1F0 == 0x100 { // sbcd
        panic("sbcd")
    } else if inst & 0x0C0 == 0x0C0 { // divu,divs
        is_signed = bool(inst & 0x100)
        ea = address_by_mode(inst & 63, 2)
        divisor = read(2, ea)
        if is_signed: divisor = signed(2, divisor)
        }
        if divisor == 0: 1/0 // div0 error
        }
        dn = (inst >> 9) & 7
        dividend = read(4, regs + dn * 4)
        if is_signed: dividend = signed(4, dividend)

        }
        // remainder is 0 or has same sign as dividend
        quotient = abs(dividend) // abs(divisor)
        if dividend < 0: quotient = -quotient
        }
        if divisor < 0: quotient = -quotient
        }
        remainder = abs(dividend) % abs(divisor)
        if dividend < 0: remainder = -remainder

        }
        if signed && !(-0x8000 <= quotient < 0x7FFF) {
            v = true
            return

        }
        if !signed && quotient > 0xFFFF {
            v = true
            return

        }
        v = false
        set_nz(quotient, 2)

        write(4, regs + dn * 4, ((remainder & 0xFFFF) << 16) | (quotient & 0xFFFF))

    } else { // or
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("or size=%11")
        }
        src = address_by_mode(inst & 63, size)
        dn = (inst >> 9) & 7
        dest = regs + dn * 4 + 4 - size

        if inst & 0x100: src, dest = dest, src

        }
        datum = read(size, src) | read(size, dest)
        write(size, dest, datum)
        set_nz(datum, size)
        v = false
        c = false
    }
}

func line9D(inst uint16) { // sub,subx,suba//add,addx,adda: very compactly encoded
//    global x, n, z, v, c
    sign = 1 if inst & 0x4000 else -1
    if inst & 0x0C0 == 0x0C0 { // suba,adda
        size = 4 if inst & 0x100 else 2
        ea = address_by_mode(inst & 63, size)
        an = (inst >> 9) & 7
        result = signed(4, read(4, regs + 32 + an * 4)) + sign * signed(size, read(size, ea))
        write(4, regs + 32 + an * 4, result)

    } else if inst & 0x130 == 0x100 { // subx,addx: only two addressing modes allowed
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("subx/addx size=%11")
        }
        mode = 32 if inst & 8 else 0 // either -(Ax),-(Ay) or Dx,Dy
        src = address_by_mode(mode | (inst & 7), size)
        dest = address_by_mode(mode | ((inst >> 9) & 7), size)

        if sign == 1 {
            result := add_then_set_vc(read(size, dest), read(size, src) + x, size)
        } else {
            result = sub_then_set_vc(read(size, dest), read(size, src) + x, size)
        }
        write(size, dest, result)
        x = c
        old_z = z; set_nz(result, size); z &= old_z

    } else { // sub,add
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("sub/add size=%11")
        }
        src = address_by_mode(inst & 63, size)
        dn = (inst >> 9) & 7
        dest = regs + dn * 4 + 4 - size

        if inst & 0x100: src, dest = dest, src // direction bit does a swap
        }
        if sign == 1 {
            result = add_then_set_vc(read(size, dest), read(size, src), size)
        } else {
            result = sub_then_set_vc(read(size, dest), read(size, src), size)
        }
        x = c
        set_nz(result, size)
        write(size, dest, result)
    }
}

func lineB(inst uint16) { // cmpa,cmp,cmpm,eor
//    global x, n, z, v, c
    if inst & 0x0C0 == 0x0C0 { // cmpa
        size = 4 if inst & 0x100 else 2 // size of ea but An is always .L
        ea = address_by_mode(inst & 63, size)
        an = (inst >> 9) & 7
        result = sub_then_set_vc(signed(4, read(4, regs + 32 + an * 4)), signed(size, read(size, ea)), 4)
        set_nz(result, 4)

    } else if inst & 0x100 == 0x000 { // cmp
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("cmp size=%11")
        }
        dn = (inst >> 9) & 7
        dest = regs + dn * 4 + 4 - size
        src = address_by_mode(inst & 63, size)
        result = sub_then_set_vc(read(size, dest), read(size, src), size)
        set_nz(result, size)

    } else if inst & 0x38 == 0x08 { // cmpm (Ay)+,(Ax)+
        size := (1, 2, 4, nil)[(inst >> 6) & 3]
        if size == 0 {panic("cmpm size=%11")
        }
        src = address_by_mode(24 | (inst & 7), size) // (An)+ mode
        dest = address_by_mode(24 | ((inst >> 9) & 7), size)
        result = sub_then_set_vc(read(size, dest), read(size, src), size)
        set_nz(result, size)

    } else { // eor
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("eor size=%11")
        }
        dn = (inst >> 9) & 7; src = regs + 4 * dn + 4 - size
        dest = address_by_mode(inst & 63, size)

        result = read(size, dest) ^ read(size, src)
        v = false
        c = false
        set_nz(result, size)
        write(size, dest, result)
    }
}

func lineC(inst uint16) {
    if inst & 0xC0 == 0xC0 { // mulu,muls
        is_signed = bool(inst & 0x100)
        src = address_by_mode(inst & 63, 2) // ea.w
        dest = regs + ((inst >> 9) & 7) * 4 // dn.l

        m1 = read(2, dest+2); m2 = read(2, src)
        if is_signed {
            m1 = signed(2, m1); m2 = signed(2, m2)
        }
        datum = m1 * m2
        set_nz(datum, 4)
        v = false
        c = false
        write(4, dest, datum) // write dn.l

    } else if inst & 0x1F0 == 0x100 { // abcd
        panic("abcd")
    } else if inst & 0x1F8 in (0x140, 0x148, 0x188) { // exg
        rx = (inst >> 9) & 7
        ry = inst & 7
        if inst & 0x1F8 == 0x148 { // Ax,Ay
            rx |= 8; ry |= 8 // bump the addressing mode from Dn to An
        }
        if inst & 0x1F8 == 0x188 { // Dx,Ay
            ry |= 8

        }
        rx = address_by_mode(rx, 4)
        ry = address_by_mode(ry, 4)

        x := read(4, rx)
        y := read(4, ry)
        write(4, rx, y)
        write(4, ry, x)

    } else { // and
        size = []int{1, 2, 4, 0}[(inst >> 6) & 3]
        if size == 0 {panic("and size=%11")
        }
        dn := (inst >> 9) & 7; dest = address_by_mode(dn, size)
        src := address_by_mode(inst & 63, size)

        if inst & 0x100: src, dest = dest, src // direction bit

        }
        result := read(size, src) & read(size, dest)
        set_nz(result, size)
        v := c = false
        write(size, dest, result)
    }
}

func lineE(inst uint16) {
//    global x, n, z, v, c
    size = []int{1, 2, 4, 0}[(inst >> 6) & 3]

    if size == nil { // single-bit shift on a memory address
        size = 2
        kind = (inst >> 9) & 3
        dest = address_by_mode(inst & 63, size)
        by := 1
    } else {
        kind = (inst >> 3) & 3
        dest = regs + (inst & 7) * 4 + 4 - size // dn
        if inst & 0x20 {
            by = read(1, regs + ((inst >> 9) & 7) * 4 + 3) % 64
        } else {
            by = (inst >> 9) & 7
            by = ((inst >> 9) & 7) || 8

        }
    }
    isleft = bool(inst & 0x100)

    v = false // clear V if msb never changes
    c = false // clear C if shift count is zero

    result = read(size, dest)
    mask = (1 << (size * 8)) - 1
    msb = 1 << (size * 8 - 1)
    numbits = size * 8
    if kind == 0 { // asl/asr
        if isleft {
            for i := range(by) {
                newresult = (result << 1) & mask
                x = c = bool(result & msb) // shifted-out bit
                if newresult & msb != result & msb: v = 1 // set V if sign ever changes
                }
                result = newresult
            }
        } else {
            for i := range(by) {
                newresult := result >> 1
                x = c = bool(result & 1) // shifted-out bit
                newresult |= result & msb // replicate sign bit
                result = newresult
            }
        }
    } else if kind == 1 { // lsl/lsr
        v = false
        if isleft {
            for i := range(by) {
                newresult = (result << 1) & mask
                x = c = bool(result & msb) // shifted-out bit
                result = newresult
            }
        } else {
            for i := range(by) {
                newresult = result >> 1
                x = c = bool(result & 1) // shifted-out bit
                result = newresult
            }
        }
    } else if kind == 2 { // roxl/roxr
        v = false
        if isleft {
            for i := range(by) {
                newresult = (result << 1) | x
                x = c = bool(newresult >> numbits)
                result = newresult & mask
            }
        } else {
            for i := range(by) {
                newresult = (result >> 1) | (x * msb)
                x = c = bool(result & 1)
                result = newresult & mask
            }
        }
    } else if kind == 3 { // rol/ror
        v = false
        if isleft {
            for i := range(by) {
                result = (result << 1) | (result >> (numbits - 1))
                c = bool(result & 1)
            }
        } else {
            for i := range(by) {
                c = bool(result & 1)
                result = (result << (numbits - 1)) | (result >> 1)

            }
        }
    }
    set_nz(result, size)

    write(size, dest, result)
}

func run_m68k_interpreter() {
    for {
        var inst = (uint16(mymem[pc]) << 8) | mymem[pc+1]
        pc = pc + 2
        switch inst >> 12 {
        case 0:
            line0(inst)
        case 1, 2, 3:
            line123(inst)
        case 4:
            line4(inst)
        case 5:
            line5(inst)
        case 6:
            line6(inst)
        case 7:
            line7(inst)
        case 8:
            line8(inst)
        case 9, 0xd:
            line9D(inst)
        case 0xa, 0xf:
            lineAF(inst)
        case 0xb:
            lineB(inst)
        case 0xc:
            lineC(inst)
        case 0xe:
            lineE(inst)
        }
    }
}

// debug_segment_list = []
// func roughly_where_we_are(pc) {
//     retval = f"{pc:x}"
//
//     for seg_num, start, end in reversed(debug_segment_list) {
//         if start <= pc < end {
//             retval += f" seg{seg_num:x}+{pc-start:x}"
//
//             for i := range(pc, end, 2) {
//                 if i + 4 >= len(mem): break
//                 }
//                 if mem[i:i+2] in (b"Nu", b"\x4e\xd0") {
//                     if 0x81 <= mem[i+2] < 0xc0 {
//                         strlen := mem[i+2] & 0x7f
//                         trystring := mem[i+3:i+3+strlen]
//                         }
//                         if len(trystring) == strlen && all(0x20 < x < 0x7f for x in trystring) {
//                             retval += f' {trystring.decode("mac_roman")}-{i+2-pc:x}'
//                         }
//                     }
//                     break
//
//                 }
//             }
//         }
//     }
//     if mem[pc:pc+2] == b"\xaa\x69" {
//         retval += " callback:" + my_callbacks[-1].__name__
//
//     }
//     retval += f" {mem[pc:pc+2].hex()}"
//
//     return retval
// }
//
// func print_state() {
//     for _, i := range (regs, regs+32) {
//         fmt.Print(" ".join(mem[r:r+4].hex() for r := range(i, i+32, 4)))
//
//     }
//     sp := read(4, regs + 32 + 7 * 4)
//     fmt.Print("sp:", " ".join(mem[r:r+2].hex() for r := range(sp, sp+20, 2)),
//         "ccr:", "xX"[x]+"nN"[n]+"zZ"[z]+"vV"[v]+"cC"[c])
//
// }

//#######################################################################
// SANE -- blobs from Mac Plus ROM
//#######################################################################

pack4 = bytes.fromhex(""'
600A 0000 5041 434B 0004 0002 4E56 FFFE 48E7 FFF8 307C 0A4A 6026 00E1 00E1 00E1
00E1 00C1 00C1 00E1 0061 0161 00A0 00A0 00A0 00A1 00A0 0041 4E56 FFFE 48E7 FFF8
3C2E 0008 7E1E CE46 0886 0000 6600 09EA 08A8 0007 0001 8C7B 70C2 7060 C028 0001
EE58 8C40 0806 0008 6714 3006 0246 00FF 0240 3800 3200 E749 E658 8C40 8C41 4846
4282 2602 3C02 0806 0017 670E 2006 4840 EE58 266E 000A 6100 00CA 0806 0016 6720
2244 2445 2F0C E21E D442 3003 D643 D640 2006 4840 ED58 266E 000E 6100 00A6 265F
E306 6804 08C6 0006 E216 3007 4287 487A 002E 4A42 6600 01DC 303B 0006 4EFB 00FA
04BC 04B8 0556 05F6 070C 070C 068E 0778 07EC 0854 0780 078C 0930 08FA 0940 0806
0015 670A 2006 4840 EE58 6100 03C0 4847 3010 4206 8046 30C0 E05E C046 6718 3F07
3F00 4857 4CEE 000F 0006 48E7 F000 548F 2050 4E90 2E1F 0806 0010 6706 7010 720E
6004 700C 720A 3D40 FFFE 2DAE 0004 1000 2C56 44C7 4CDF 1FFF DED7 4E75 7014 7212
60E4 4284 2A04 0240 000E 303B 0006 4EFB 00F2 0068 00F2 00C8 0068 001E 0026 002C
700F 3813 4844 602A 701F 2813 6024 703F 2813 2A2B 0004 6704 4A84 601A 0C84 8000
0000 660E 387C 7FFF 7814 4844 08C4 001E 606A 4A84 673A 6A08 08C6 0007 4485 4084
0640 3FFF 4A84 6B3A 6030 3013 6A08 08C6 0007 0880 000F 282B 0002 2A2B 0006 0C40
7FFF 6724 4A84 6B1A 4A84 660A 4A85 6606 99CC 5443 4E75 08C3 001F 5340 DA85 D984
6AF8 48C0 2840 4E75 387C 7FFF 0884 001F 4A84 6608 4A85 6604 5843 4E75 08C4 001E
6604 08C6 0008 5442 4E75 4280 2813 DC06 D884 E216 E19C 1004 6712 183C 0001 E29C
0C00 00FF 67C2 0640 3F80 60B6 303C 3F81 E29C 6094 2813 6A04 08C6 0007 2A2B 0004
E19D E79D E19C E99C 3004 E28C 0244 F800 3205 0241 07FF 8841 0245 F800 0240 07FF
6608 303C 3C01 6000 FF60 0C40 07FF 6700 FF78 08C4 001F 0640 3C00 6000 FF66 08C6
0008 4840 08C0 001E 2800 4285 387C 7FFF 6024 0C42 0002 671E 0C42 0004 6710 203C
00FF 0000 2209 C280 C084 B280 6F08 E31E 284B 2A0A 2809 0806 001A 671E 4284 7A01
E29D 0806 0018 6608 0806 0019 6608 4845 08C6 0008 4E75 C945 4E75 0806 0015 661A
0806 0014 6606 7002 6000 04E2 7001 0886 0008 6602 5240 6000 0712 0806 001F 6706
7A00 1805 600A 0806 001E 6704 0245 F800 2004 0880 001E 8085 6608 7815 4844 08C4
001E 4E75 DC06 2809 2A0A 284B 4E75 0C40 0042 6304 303C 0042 E28C E295 E257 55C1
8E01 5340 66F2 4E75 4A84 660C 4A85 6608 4A47 6604 99CC 4E75 4A84 6008 538C DE47
DB85 D984 6AF6 4A86 6B00 00C6 0806 001E 6600 00F6 97CB 611A 4281 7401 0805 0000
6120 367C 7FFE 616C 6706 284B 78FF 2A04 4E75 200B 908C 6E02 4E75 08C6 0009 284B
608C 56C0 4A47 6606 0886 0009 4E75 08C6 000C 0810 0005 670E 0810 0006 6702 4E75
4A06 6A1E 4E75 0810 0006 6610 0C47 8000 6402 4E75 620C 4A00 6608 4E75 4A06 6B02
4E75 08E8 0007 0001 DA82 D981 6404 E294 528C 4E75 B7CC 6D04 4240 4E75 08C6 000A
08C6 000C 387C 7FFF 4284 2A04 1210 0201 0060 6602 4E75 0C01 0060 6604 4A01 4E75
4A06 6B06 0C01 0020 4E75 0C01 0040 4E75 367C 3F81 6100 FF5C 4A85 56C0 8E00 D804
E257 8E04 4285 4204 223C 0000 0100 4282 0804 0008 6100 FF4C 367C 407E 6196 6706
284B 78FF 4204 4E75 367C 3C01 6100 FF24 303C 07FF C045 0245 F800 EB48 E24F 6404
08C7 0000 8E40 4281 243C 0000 0800 0805 000B 6100 FF0E 367C 43FE 6100 FF58 670A
284B 78FF 2A3C FFFF F800 4E75 0240 000E 303B 0010 266E 000A 360C 0C43 7FFF 4EFB
00EC 0032 0086 0060 0000 0024 0028 002C 3685 4E75 2685 4E75 26C4 2685 4E75 0806
0009 6718 4A43 6714 4A84 660E 4A85 6604 4283 6008 5343 DA85 D984 6AF8 4A06 6A04
0643 8000 36C3 26C4 2685 4E75 6606 363C 4080 6008 4A43 6604 363C 3F81 0443 3F80
D884 6502 5343 8843 E09C DC06 E294 2684 4E75 6606 363C 4400 6008 4A43 6604 363C
3C01 0443 3C00 4A84 6B02 5343 203C 0000 07FF C084 8A80 E09D E69D 0244 F800 D884
8843 E09C E69C DC06 E294 26C4 2685 4E75 0A06 00A0 363B 3006 4EFB 30FA 001A 0080
FDB8 0088 0066 FDB8 FDB0 FDB0 008C 2604 2809 CB8A 300C 904B 6712 6E0C CB8A C943
4440 284B DC06 E206 6100 FD84 0806 0005 6612 DA8A D983 6408 E294 E295 E257 528C
6000 FDA4 4606 9A8A 9983 6714 6408 4447 4085 4084 4606 6000 FD80 0806 0005 6712
99CC 4206 0810 0006 6708 0810 0005 6602 4606 4E75 2A0A 2809 284B DC06 6000 FD68
0806 0005 6602 4E75 7002 6000 FC72 E51E 7008 363B 3006 4EFB 30F6 001E FD1E FD1E
FD18 FD1E FC70 FD18 FC70 FD1E D9CB 98FC 3FFE 48E7 0680 2644 4284 2A04 2044 2217
6606 200A 6728 6012 200A 6130 4A85 56C7 2A04 4284 2217 2009 6122 220B 200A 611C
4A45 56C0 8E00 4845 8E45 2A04 2808 220B 2009 6108 4CDF 0141 6000 FCDE 3401 3601
C4C0 3C00 4840 4841 CCC1 C6C0 C2C0 4842 4280 3002 D680 D686 3403 4842 4243 D743
4843 D283 DA82 D981 6402 5248 4E75 E51E 7004 363B 3006 4EFB 30F6 0034 001E 002C
FC78 FBD0 FC78 FC78 FC78 FBD0 08C6 000B 387C 7FFF 4284 2A04 4E75 99CC 280C 2A04
4E75 C74C 99CB D8FC 3FFF 7041 4A86 6A02 7021 B889 6602 BA8A 6304 5240 538C 2209
240A 2604 2445 611A 4A86 6A04 2805 4285 44FC 0010 E294 E295 E257 8282 56C7 6000
FC46 4284 2A04 600A DA85 D984 D482 D381 6508 B283 6602 B48A 6506 5205 948A 9383
5340 66E4 4E75 426F 0006 DC06 7009 0286 3FFF FFFF 363B 3006 4EFB 30EC 0028 FB38
0030 FBE0 FB38 FBE0 FB38 FB38 FB38 200B 5480 908C 6E08 2809 2A0A 284B 6042 538C
2209 240A 2604 2445 6198 0805 0000 671C 4A81 660A 4A82 6606 0805 0001 6706 0846
0007 5445 C58A C343 948A 9383 E20D 0806 0006 6702 4405 4885 3F45 0006 2801 2A02
6000 FB86 363B 301C 4EBB 30FA 0C40 0002 660A 0806 0011 6704 08C6 0008 3E00 4847
4E75 0048 0030 0034 0034 0044 0034 0030 0030 003E DC06 4606 7000 4A06 6B02 7019
4E75 0806 0005 66F0 7004 4E75 0806 0005 66E6 B7CC 6E0E 6D0E B3C4 6208 6508 B5C5
67E6 6502 4606 7019 4A06 6A02 7000 4E75 4A43 6700 FB32 4E75 4A43 6702 4E75 6134
3450 6012 4A43 6702 4E75 6128 3450 08D0 0006 08D0 0005 6100 FB0E 308A 4A84 6B12
6608 4A85 6604 99CC 6008 538C DA85 D984 6AF8 4E75 4A86 6B0C 0806 001E 670C 7034
6000 000A 7017 6000 0004 703F 0640 3FFF 3200 908C 6E02 4E75 3841 6100 FA92 44FC
0000 4E75 0806 001A 6786 0C43 0002 6602 4E75 0C43 0004 6604 78FF 6010 61CC 6F0C
4281 7401 0805 0000 6100 FAD8 7201 E299 0806 0018 6608 0806 0019 6626 4841 4A84
6614 BA81 6210 6504 4A06 6A0A 4A06 6A04 4485 4084 4E75 2A01 08C6 0008 0886 000C
4E75 4A84 6AE6 4285 2801 60EC 0C43 0002 6602 4E75 7001 4A06 6B00 F964 0C43 0004
6602 4E75 300C E240 40C1 6402 5240 0640 1FFF 3840 2246 2447 2E05 2C04 4283 2403
DE87 DD86 D743 44C1 6506 DE87 DD86 D743 2002 2802 7A03 5343 7241 3641 4281 DE87
DD86 D783 D582 D341 DE87 DD86 D783 D582 D341 DA85 D984 D140 5345 9685 9584 9340
6408 D685 D584 D340 6002 5445 C18B 5340 C18B 66CA 2C09 2E0A 8282 8283 6602 5345
7203 E248 E294 E295 E257 5341 66F4 6000 F9B6 4206 0C43 0002 660E 0046 0880 387C
7FFF 4284 2A04 4E75 0C43 0004 6602 4E75 4285 98FC 3FFF 280C 6A06 0006 0080 4484
387C 401E 6000 F962 4A43 6702 4E75 266E 000E D8D3 6000 F970 7005 4A83 6B0A 670A
5540 0C43 0004 6702 5240 4A06 6702 4440 266E 000A 3680 4E75 4CEE 0600 000A 3E3B
7006 4EFB 70F4 002A 0034 0038 003E 01F2 055A 0044 004A 005E 0072 01D8 002E 01E4
0052 3091 6020 3290 4250 601A 3290 6016 2151 0002 6010 22A8 0002 600A 0851 0007
6004 0891 0007 6000 F6BA 3011 0110 56C0 4400 1280 60F0 0811 0007 6706 08D2 0007
6004 0892 0007 6000 F694 9EFC 0016 284F 3A06 0245 3800 5246 4846 4246 2F0A 486C
000A 700E 8045 3F00 6100 F56E 4294 42AC 0004 426C 0008 4854 486C 000A 3F3C 0008
6100 F556 56C4 2F09 4854 3F00 6100 F54A 4854 486C 000A 3F3C 0008 6100 F53C 6822
4854 486C 000A 3F3C 0004 6100 F52C 486C 000A 2F0A 7010 8045 3F00 6100 F51C 6000
0080 677C 6408 0812 0007 6708 607A 0812 0007 6774 0805 000C 6704 5292 6034 0805
000B 670A 52AA 0004 6402 5292 6024 52AA 0006 641E 52AA 0002 6418 E4EA 0002 5252
0C52 7FFF 6706 0C52 FFFF 6606 08AA 0007 0002 2F0A 486C 0014 701C 8045 3F00 6100
F4B8 322C 0014 6A02 4441 0C41 0003 6606 0046 1400 600A 0C41 0005 6704 0046 1200
DEFC 0016 6000 F568 0805 000C 6710 4A04 6608 0852 0007 5292 60B8 5392 60B4 0805
000B 6718 4A04 660A 0852 0007 526A 0006 60A0 53AA 0004 6402 5392 6096 4A04 660A
0852 0007 526A 0008 6088 53AA 0006 641C 53AA 0002 6B12 4A52 6712 0C52 8000 670C
066A 8000 0002 6002 6402 5352 6000 FF64 4846 4246 3011 5040 01C6 600A 4846 3C3C
1F00 CC50 3091 6000 F4E6 9EFC 0014 264F 49EA 0004 362A 0002 4846 4284 2A04 2E04
7413 1C1C 6768 1014 0C00 0049 6754 0C00 004E 6654 142C FFFF 528C 5302 7008 0C02
0004 6C04 5900 D002 610A 2805 4285 7008 6102 601E E99D 5302 6B12 121C 0C01 0039
6F04 0601 0009 0201 000F 8A01 5340 66E4 4E75 0284 7FFF FFFF 6608 7815 4844 08C4
001E 303C 7FFF 6008 0C00 0030 6628 4280 4A12 6704 08C0 000F 6100 0074 6000 0052
DA85 D984 2205 2004 DA85 D984 DA85 D984 DA81 D980 4E75 61E8 700F C01C DA80 4240
D980 5306 6716 5302 66EC 1E3C 000F CE14 600E 0C43 001B 6F08 61C6 5343 5342 66F2
303C 403E 4A12 6704 08C0 000F 6120 6128 4846 0246 3800 0046 0010 4853 2F2E 000A
3F06 6100 F314 DEFC 0014 6000 F410 36C0 26C4 2685 5D8B 4E75 1C10 1F06 0206 0060
670E 7240 4A13 6A0C 0C06 0040 670C 600E 1086 6014 0C06 0020 6604 0A01 0060 4A43
6A04 0A01 0060 1081 43EB 000A 6100 00F2 0010 0060 486B 000A 4853 7004 4A43 6A02
7006 3F00 6100 F2B2 0810 0004 56C0 4A07 56C7 8007 1210 0201 001F 821F 1081 4400
673E 08D0 0004 4A06 6728 0C06 0060 6730 2F3C 0000 0001 42A7 3F13 5357 6502 6802
5257 4857 4853 4267 6100 F26E DEFC 000A 600E 3013 5240 D040 6706 08EB 0000 0009
4E75 000E 0000 402D B5E6 20F4 8000 0000 001B 0000 4058 CECB 8F27 F420 0F3A 0037
0001 40B5 D0CF 4B50 CFE2 0766 006C 0001 4165 DA01 EE64 1A70 8DEA 00CE FFFF 42AB
9F79 A169 BD20 3E41 019C 0001 4557 C6B0 A096 A952 02BE 0338 0001 4AB0 9A35 B246
41D0 5953 0670 0001 5561 B9C9 4B7F A8D7 6515 0CE0 0001 6AC4 86D4 8D66 26C2 7EEC
7209 3003 6A02 4440 0C40 000F 6564 0C40 1388 6504 303C 1388 45FA FFD8 B052 654A
905A 4A52 673E 08D0 0004 0810 0006 6706 4A52 6F30 600A 0810 0005 6728 4A52 6C24
2F0A 3F2A 000A 4A5A 6A04 5257 6002 5357 2F2A 0004 2F12 244F 6120 508F 548F 245F
548A 6004 548A 6112 598A 94FC 000E 5341 66AA C0FC 000A 45FB 0024 4A81 6A0E 4852
4851 3F3C 0004 6100 F150 4E75 32DA 22DA 2292 5D89 5D8A 08C1 001F 4E75 3FFF 8000
0000 0000 0000 4002 A000 0000 0000 0000 4005 C800 0000 0000 0000 4008 FA00 0000
0000 0000 400C 9C40 0000 0000 0000 400F C350 0000 0000 0000 4012 F424 0000 0000
0000 4016 9896 8000 0000 0000 4019 BEBC 2000 0000 0000 401C EE6B 2800 0000 0000
4020 9502 F900 0000 0000 4023 BA43 B740 0000 0000 4026 E8D4 A510 0000 0000 402A
9184 E72A 0000 0000 402D B5E6 20F4 8000 0000 3006 ED58 4282 2602 2C02 2E02 264A
6100 F1C0 300C 286E 0012 9EFC 001E 264F E31E 1286 137C 0001 0004 4A42 6734 5889
12FC 0011 12FC 004E 610C 2805 6108 92FC 0016 6000 01DC 7008 E99C 720F C204 0001
0030 0C01 0039 6F02 5E01 12C1 5340 66E8 4E75 4A43 6712 7030 0C43 0002 6702 7049
1340 0005 6000 01AA 2204 3200 0441 3FFF 4841 D241 4A40 6A08 E28C E295 5240 6BF8
E25E 8046 6100 FD08 4284 4A14 662C 203C 4D10 4D42 4A81 6A02 5240 2800 4844 C8C1
4244 4844 4841 3A01 CBC0 4845 48C5 D885 4840 C1C1 D880 4844 5244 6100 0084 9644
226E 000A 3343 0002 4469 0002 3753 0014 276B 0002 0016 276B 0006 001A 6100 FCBA
4853 3F3C 0014 6100 EFB0 43EB 000A 4A14 6604 614C 6002 7613 6100 FDC6 3013 0880
000F B051 6612 202B 0002 B0A9 0002 6608 202B 0006 B0A9 0006 6540 4A14 6614 5244
36AB 0014 276B 0016 0002 276B 001A 0006 6088 226E 000A 137C 003F 0005 6000 00D2
362C 0002 4A14 6610 4A43 6F0A 0C43 0013 6F06 7613 6002 7601 4E75 4A14 6636 61E0
5343 6100 FD5C 3013 0880 000F B051 6612 202B 0002 B0A9 0002 6608 202B 0006 B0A9
0006 6410 3691 2769 0002 0002 2769 0006 0006 4E71 226E 000A 3613 222B 0002 660A
137C 0030 0005 6000 0068 242B 0006 0883 000F 0443 403E 6708 E289 E292 5243 6BF8
263C 8AC7 2304 247C 89E8 0000 7041 6100 F5D2 5285 D980 5889 2449 421A 7C13 DA85
D984 D140 2605 2404 3200 DA85 D984 D140 DA85 D984 D140 DA83 D982 D141 4A40 670C
0080 0000 0130 14C0 4200 5211 5306 66CE DEFC 001E 6000 EFA6
""')

pack5 = bytes.fromhex(""'
600A 0000 5041 434B 0005 0001 4E56 FFD0 48E7 FFF8 4283 2078 0ACC 1D50 FFD0 08D0
0007 47EE 0004 41EE 0008 3618 6A48 2858 2810 0803 000E 6618 2093 36BC 000A 2A0C
740F 0C43 8010 6706 2044 6110 3400 6032 5888 2A10 2093 36BC 000E 60EC 4850 486E
FFFC 3F3C 001C A9EB 302E FFFC 6A06 4440 0040 8000 4E75 740F 2850 2093 36BC 0006
2A0C 2045 61D6 3200 486E FFFE 3F3C 0017 A9EB 5702 6C10 2045 224C 614C 2F04 4854
4267 A9EB 600E 5701 6C0E 2F05 4854 3F3C 000E A9EB 6000 030C 3003 0240 00FE 303B
0006 4EFB 00F4 013A 013A 013A 013A 0326 0326 03D6 03D6 04E6 0608 06F8 078E 08E8
093A 0A52 0B3C 0C80 22D8 22D8 3290 5189 4E75 3019 4851 4850 3F3C 000E A9EB 4852
4850 3F3C 0004 A9EB 5089 5489 4851 4850 4267 A9EB 5340 6EE6 5089 5489 4E75 7001
600A 7002 6006 7000 6002 7004 558F 4857 3F3C 0003 A9EB 0197 4857 3F3C 0001 A9EB
548F 4A00 4E75 7002 600E 7001 600A 7003 6006 7000 6002 7004 3F00 4857 3F3C 0015
A9EB 60DC 7003 600E 7001 600A 7002 6006 7000 6002 7004 3F00 4857 3F3C 001B A9EB
1017 60BC 3D7C 7FFF FFFA 47EE FFE6 2729 0006 2729 0002 3711 0893 0007 6144 6C16
487A 0C9A 4853 3F3C 0002 A9EB 4A11 6A04 446E FFFA 611E 612A 6C06 487A 0C80 6002
4853 486E FFFA 3F3C 2010 A9EB 4A11 6A04 446E FFFA 486E FFFA 4850 3F3C 2018 A9EB
4E75 486E FFDC 487A 0C54 3F3C 0008 A9EB 4E75 5301 6A0C 4A41 6A00 01A0 7024 6000
01B0 0803 0002 660A 4A01 671C 4A41 6BEC 605A 4A01 6700 01AC 4854 487A 0BF8 3F3C
0008 A9EB 62D6 6504 6000 0176 204C 43EE FFF0 6100 FEB4 487A 0BD2 4854 4267 A9EB
487A 0C7C 4854 3F3C 0008 A9EB 6F1E 4854 487A 0C62 3F3C 0008 A9EB 6310 2049 224C
6100 FE86 6100 007E 6000 0064 204C 43EE FFF0 6100 FE74 4851 3F3C 001A A9EB 0851
0007 204C 6100 FEFE 0851 0007 487A 0C26 4854 3F3C 0008 A9EB 6F00 001A 487A 0B6C
486E FFF0 4267 A9EB 487A 0B88 4854 3F3C 0006 A9EB 487A 0B54 4854 3F3C 0002 A9EB
6100 0022 486E FFF0 4854 4267 A9EB 0803 0001 660C 487A 0BFC 4854 3F3C 0004 A9EB
6000 00E0 4854 486E FFFA 3F3C 001C A9EB 302E FFFA 6A02 4440 5940 6602 4E75 5340
670A 487A 0BCE 6100 FE42 6064 204C 43EE FFE6 6100 FDD4 487A 0B1A 4854 4267 A9EB
4854 4851 3F3C 0006 A9EB 2049 4851 4851 43EE FFDC 6100 FDB2 3F3C 0004 A9EB 204C
43FA 0A20 45EE FFE6 6100 FDA8 486E FFDC 4854 3F3C 0004 A9EB 41EE FFDC 43FA 0A42
45EE FFE6 6100 FD8C 6100 FDB4 486E FFDC 4854 3F3C 0006 A9EB 6100 FDDC 4E75 41FA
0A9E 6024 41FA 0AA2 601E 41FA 0A7E 6018 41FA 0A82 6012 6100 FDB6 41FA 0AAA 6008
6100 FDAC 41FA 0AAA 224C 6100 FD3C 6012 0080 7FFF 4000 28C0 429C 4254 518C 6100
FD92 486E FFFE 3F3C 0019 A9EB 2078 0ACC 10AE FFD0 4CDF 1FFF 4E5E DED7 4E75 5301
67A8 6E06 4A41 6B96 60D8 0803 0001 6704 613A 601E 6100 0058 6100 FD72 6714 6100
FD56 4A41 6A94 6100 FD1A 6100 FD3E 6000 FF6E 6100 010E 487A 09F2 4854 4267 A9EB
204C 43EE FFF0 6100 FD5C 6096 204C 43EE FFF0 6100 FCB4 4851 3F3C 0014 A9EB 6100
FCEA 4851 4854 3F3C 0002 A9EB 4E75 204C 43EE FFF0 6100 FC92 487A 0A78 2F17 2F17
4854 3F3C 000C A9EB 4854 4851 3F3C 0002 A9EB 4851 3F3C 0006 A9EB 4851 3F3C 0014
A9EB 4854 3F3C 0006 A9EB 6000 FC9E 5301 6E0E 6700 0008 4A41 6B00 FEF6 6000 FF24
5301 0803 0001 6720 4A01 6716 487A 0A24 4854 3F3C 0004 A9EB 6100 FC90 6100 FC98
605C 6100 FF68 601E 4A01 66EC 6180 6100 FC9C 6712 6100 FC80 4A41 6A00 FEBE 6100
FC42 6000 FEAC 6100 003A 487A 0932 486E FFF0 3F3C 0008 A9EB 6724 487A 090E 4854
4267 A9EB 204C 43EE FFF0 6100 FC78 487A 08FA 4854 3F3C 0002 A9EB 6100 FC02 6000
FEA2 487A 08FA 4854 3F3C 0008 A9EB 6602 4E75 43EE FFDC 204C 6100 FBAE 4851 4851
3F3C 0004 A9EB 41EE FFE6 43FA 0874 45EE FFDC 6100 FB9E 486E FFE6 4854 3F3C 0004
A9EB 41EE FFE6 43FA 0882 45EE FFDC 6100 FB82 4854 486E FFE6 3F3C 0002 A9EB 487A
08B2 4854 3F3C 0004 A9EB 486E FFE6 4854 3F3C 0006 A9EB 6100 FBBE 6000 FB82 2044
3410 6700 FDE6 5301 6E26 E242 6504 0894 0007 D542 6A00 FE0C 4A01 6A0A 4A14 6A00
FDBE 6000 FDC0 4A14 6A00 FDCC 6000 FDD2 3002 6A02 4440 0C40 00FF 6206 612C 6000
FDE2 3F14 0894 0007 2F04 486E FFF0 2817 3F3C 200E A9EB 6100 0098 301F 6A08 E242
6404 08D4 0007 6000 FDBA 204C 43EE FFE6 6100 FAD6 614E 4A42 6B08 2049 224C 6000
FAC8 41FA 07E6 43D4 6100 FABE 6100 FB3A 6614 6100 FB38 660E 486E FFF0 4854 3F3C
0006 A9EB 4E75 6100 FADA 6100 FAD2 486E FFE6 4854 3F3C 0006 A9EB 3002 6A02 4440
6102 60B6 41FA 07A4 43EE FFF0 6100 FA7A 600A 4854 4854 3F3C 0004 A9EB E248 640A
4854 4851 3F3C 0004 A9EB 4A40 66E4 4E75 204C 43EE FFE6 6100 FA50 4851 3F3C 0002
6100 F95E 2F04 4851 3F3C 0004 A9EB 4851 3F3C 000A 6100 F94A 2049 224C 6000 FA2A
4A41 6B14 6100 0068 6000 FCF8 588F 6100 FA5A 7025 6000 FCDA 4A02 67F2 2044 43EE
FFF0 6100 FA04 4851 3F3C 0014 A9EB 6100 FA84 66DA 487A 073C 4851 3F3C 0006 A9EB
4851 3F3C 0014 A9EB 486E FFFA 3F3C 0003 A9EB 6100 FA16 0894 0007 6100 0012 082E
0004 FFFA 6704 0854 0007 6000 FC96 5301 661C 5302 6796 4A42 6A0A 6100 FA12 41FA
0706 6004 41FA 06D8 224C 6000 F99C 6A08 5302 6618 6000 FF76 5302 6A10 487A 06AC
4854 3F3C 0008 A9EB 6700 FF62 3C02 2F04 486E FFFC 3F3C 2010 A9EB 6100 F9F4 56C7
6100 F9A4 6100 F9EE 56C1 8E01 6612 342E FFFC 3002 6A02 4440 0C40 00FF 6F00 FE6C
6100 F988 6100 FEEA 4A06 6B00 F97E 4E75 487A 0662 2F05 3F3C 0008 A9EB 6D00 0082
6E00 0016 5302 6604 6000 FBC0 2044 4A10 6B00 FBC4 6000 FBA8 5302 67EC 6E0C 5301
675E B541 6BEE 6000 FBB2 43EE FFE6 2045 6100 F8F6 3011 0880 000F 0C40 3F7F 6D1C
4851 3F3C 0006 6100 F7F8 2F04 4851 3F3C 0004 A9EB 4851 3F3C 000A 6010 2F04 4851
3F3C 0004 A9EB 4851 3F3C 0008 6100 F8F0 6100 F7CE 2049 224C 6100 F8AE 6000 FB84
7026 6000 FB6C 487A 05CC 2F05 3F3C 0008 A9EB 6DEC 660E 5302 670E 4A42 6A00 FB38
6000 FB2E 5302 6604 6000 FB14 5301 6E28 6D0C 224C 2044 6100 F870 6000 FB46 4A42
6AE6 487A 0590 2F04 3F3C 0008 A9EB 67D0 6E00 FAF2 6000 FB0E 4A02 6A26 B541 6BD2
2045 43EE FFE6 6100 F840 4851 224C 41FA 055A 6100 F834 4851 3F3C 0006 A9EB 6000
FB02 43EE FFE6 2045 6100 F81E 3011 0880 000F 0C40 3F7F 6D2E 4851 3F3C 0006 6100
F720 2F04 4851 3F3C 0004 A9EB 0851 0007 0C51 4007 6D08 2045 0C50 407F 6C5C 4851
3F3C 000E 6014 2F04 4851 3F3C 0004 A9EB 0851 0007 4851 3F3C 000C 6100 F6E4 0851
0007 2F05 4851 3F3C 0006 A9EB 6100 F7F0 6100 F7F0 2049 6100 F724 5700 6606 6100
F806 6008 5500 6704 6100 F800 41EE FFE6 224C 6100 F794 6000 FA6A 2045 6100 F78A
4851 3F3C 0002 6100 F698 43EE FFDC 2044 6100 F776 487A 0494 4851 4267 A9EB 4851
43EE FFE6 4851 3F3C 0004 A9EB 0851 0007 4851 3F3C 000A 6100 F668 0851 0007 608C
7021 7400 5301 6700 FA1A 6B00 FA04 614E 0802 0000 6604 6156 6004 6100 00B8 0242
0003 5542 6B04 0854 0007 6100 F752 6100 F786 4854 486E FFFC 3F3C 001C A9EB 302E
FFFC 6A02 4440 5D40 6604 6100 F75E 6000 F9D2 7021 7401 5301 66B0 6000 F98E 487A
048C 4854 3F3C 000C A9EB D440 4E75 45FA 04EA 612E 4854 4852 3F3C 0004 A9EB 4852
4850 3F3C 0004 A9EB 4850 4854 3F3C 0002 A9EB 4E75 610C 4852 4850 3F3C 0004 A9EB
4E75 204C 43EE FFF0 6100 F69E 4851 4851 3F3C 0004 A9EB C34A 41EE FFDC 6100 F694
4850 41EE FFE6 6100 F68A 4850 3F3C 0006 A9EB 4E75 45FA 04D8 61BA 4852 4850 3F3C
0004 A9EB 487A 03DE 4852 3F3C 0008 A9EB 6E28 487A 03DA 4852 3F3C 0004 A9EB 4852
4850 3F3C 0002 A9EB 224C 6100 F63C 487A 035A 4854 4267 A9EB 4E75 0894 0007 487A
03AE 4854 3F3C 0002 A9EB 4850 204C 224A 6100 F616 487A 0398 4851 3F3C 0004 A9EB
4851 4854 3F3C 0004 A9EB 4854 3F3C 0002 A9EB 4851 4854 4267 A9EB 0854 0007 487A
0382 4854 4267 A9EB 4E75 7021 7400 5301 6B00 F89E 6700 F8AC 6100 FEE4 6134 E25A
642C 0854 0007 204C 43EE FFF0 6100 F5BA 4851 41FA 02D6 224C 6100 F5AE 4854 3F3C
0006 A9EB 6100 F61E 6704 0854 0007 6000 FE7A 45FA 044E 6100 FEDC 204C 43EE FFD2
6100 F586 4852 4851 3F3C 0004 A9EB 4851 41EE FFE6 4850 3F3C 0004 A9EB 487A 02E6
4852 3F3C 0008 A9EB 6E16 487A 02CE 4851 3F3C 0006 A9EB 4851 4850 4267 A9EB 604A
4850 204C 43EE FFDC 6100 F53E 487A 02CA 4851 3F3C 0002 A9EB 487A 02A0 4851 3F3C
0006 A9EB 4852 4851 3F3C 0004 A9EB 4851 4267 A9EB 487A 028E 4852 3F3C 0004 A9EB
4852 4851 4267 A9EB 2049 4850 4854 4267 A9EB 4E75 5301 6C10 41FA 0292 224C 6100
F4E8 4201 8354 6002 6604 6000 F7B6 0894 0007 6108 4201 8354 6000 FDB0 7400 487A
01EA 4854 3F3C 0008 A9EB 6320 74FF 204C 43EE FFF0 6100 F4B2 4851 41FA 01CE 224C
6100 F4A6 4851 3F3C 0006 A9EB 2F1C 2F1C 3F14 518C 487A 029A 4854 3F3C 0008 A9EB
6E16 45FA 0392 6100 FDDA 4850 4854 3F3C 0004 A9EB 6000 0098 204C 43EE FFF0 6100
F468 487A 0258 4851 3F3C 0004 A9EB 4851 41FA 0178 43EE FFDC 6100 F44E 4851 3F3C
0006 A9EB 487A 0164 4851 4267 A9EB 487A 015A 486E FFF0 4267 A9EB 487A 0220 4854
3F3C 0002 A9EB 204C 43EE FFD2 6100 F41A 486E FFF0 4854 3F3C 0006 A9EB 486E FFDC
4851 3F3C 0006 A9EB 45FA 030C 6100 FD54 4850 4854 3F3C 0004 A9EB 487A 01EA 4854
4267 A9EB 486E FFD2 4854 4267 A9EB 43EE FFF8 329F 231F 231F 4851 4854 3F3C 0002
A9EB 4A42 6606 0854 0007 4E75 487A 015E 4854 4267 A9EB 4E75 487A 0166 4854 3F3C
0004 A9EB 487A 0164 4854 3F3C 000C A9EB 4A14 6A0A 487A 0154 4854 4267 A9EB 6000
F662 0005 3FF6 B946 46AF F6CE 3BFF 3FF9 94F4 3221 4D61 FBEB 3FFB DC97 8903 D944
7893 3FFF 9F9E 3946 57CA 1D05 C001 CD6B C53A 46EC DC9A 4001 B483 97E7 BAE6 FCE0
0002 3FFF 8000 0000 0000 0000 C000 B818 35C3 CE0A 5E1D 3FFF FA3E DF0E DDF4 1A96
0003 BFEE FF88 90C8 1B20 9B79 3FF9 DA5C 84EF B813 272F 4003 97D3 BAEA F80F FD45
4009 AB86 71A1 95BE FF0C 0002 3FFF 8000 0000 0000 0000 4006 D548 7002 984B 5AB4
400A F775 612E 96B7 0780 3FFF 8000 0000 0000 0000 BFFF 8000 0000 0000 0000 0000
0000 0000 0000 0000 8000 0000 0000 0000 0000 4000 8000 0000 0000 0000 400D FFFE
0000 0000 0000 7FFF 0000 0000 0000 0000 FFFF 0000 0000 0000 0000 4000 C000 0000
0000 0000 3FFD 8000 0000 0000 0000 3FFE 8000 0000 0000 0000 3FFE C000 0000 0000
0000 3FFE E000 0000 0000 0000 3FFF C90F DAA2 2168 C235 3FFE C90F DAA2 2168 C235
400D 834E 0000 0000 0000 401D FFFF FFFE 0000 0000 3FFF B504 F333 F9DE 6484 3FFE
B504 F333 F9DE 6484 4000 ADF8 5458 A2BB 4A9B 3FFE B172 17F7 D1CF 79AC 3FFE 93CD
3A2C 8198 E26A 3FFA DC2A 86B1 5FDB 6462 3FFD 8930 A2F4 F66A B18A 0003 3FFF 8000
0000 0000 0000 4007 88D8 B844 A6A8 5A30 400E 8A15 8A49 0979 2320 4013 FAA9 FE26
FE63 F22B 0003 BFFE C250 86AE 43F5 CBF5 4006 9E76 EC66 BEE7 6BA2 C00C AF51 A1D6
C4C4 AA01 4011 A71B FEC4 A997 F6CE 0003 3FFF 8000 0000 0000 0000 4007 A6D9 6A95
FBDF 9685 400E C7A7 EA65 E85D 156B 4014 D26E 871C 7B17 52B2 0003 BFFB D57E 18D7
A6F6 D758 4003 E364 937E 32B3 D072 C00A A62D 4056 586B C045 4010 8C49 AF68 520F
8C7A 0003 BFFF 8000 0000 0000 0000 4005 B513 0324 83C0 0955 C009 D1FF B242 5762
E0CF 400A E190 9309 9376 66AB 0003 BFE9 B1FC 3363 D79B 2838 3FFD AA9D 71D8 3EC7
4CD1 C003 E9CD 8E24 B965 7DBB 4007 F09A 3670 9D4B 1836 0004 3FFF 98D3 1EAE 2AFA
5DAD 4003 8F79 4CD8 B3CD EFD9 4005 8E37 ED85 70B8 B368 4005 CDA4 BEA1 AC59 F3B8
4004 C405 9C55 4CF0 C656 0004 3FFF 8000 0000 0000 0000 4002 A23B 6C13 1F92 0E30
4003 C3C8 593D FA3A 486F 4003 82AE 6838 DDF5 D945 0000 0000 0000 0000 0000
""')

pack7 = bytes.fromhex(""'
600A 0000 5041 434B 0007 0001 536F 0004 6A76 2F57 0002 544F 48E7 FE40 7200 7400
7600 7800 7A00 7C1F 43E8 0001 4A80 6E0E 6B06 12FC 0030 6028 12FC 002D 4480 D080
CB05 C904 C703 C502 C301 51CE FFF2 611E 1202 611A 1203 6116 1204 6112 1205 610E
3009 9048 5300 1080 4CDF 027F 4E75 E859 6102 E959 4A46 6A06 4A01 670A 7C00 0001
0030 12C1 9201 4E75 302F 0004 2F57 0002 544F 5340 6A3E 48E7 7080 7000 7200 7600
1418 672A 0C10 002B 6708 0C10 002D 6608 7601 5288 5302 6716 720F C218 D080 D280
E588 D081 5302 66F0 4A03 6702 4480 4CDF 010E 4E75 5340 6B0A 670C 0C40 0001 670A
4E75 4EFA 0010 4EFA 02A8 4EFA 0002 7201 E299 6002 7200 4E56 0000 48E7 3F30 4CEE
0070 000C CCB8 031A 2046 2245 7C00 1C10 47F0 6000 D0D1 2C08 2244 45E9 0004 2801
0604 0030 7E39 7000 7200 7400 7600 7A00 4291 24BC 054E 3030 357C 3131 0004 6100
008A 0C00 0020 67F6 0C00 00CA 67F0 0C00 0009 67EA 0C00 002B 6708 0C00 002D 6604
5211 6166 0C00 0030 650C 621C 08C4 000F 7CFF 6156 60EE 0C00 002E 6700 0074 4A44
6A00 01B0 6000 00E4 0C00 0039 630A 4A44 6A00 0130 6000 0086 7CFF 6118 6506 0C00
0039 63F6 0C00 002E 6600 0072 611C 6500 00BA 6062 0C02 0013 6D04 6E0A 5241 528A
1480 5202 6004 8112 5241 4A84 6A06 1018 6710 6006 B7C8 6D08 1018 0C00 0030 4E75
5288 08C4 0010 4A86 6A00 0148 6000 007C 61D8 6710 6506 0C00 0039 6312 4A44 6A00
0132 6018 08C4 000F 7CFF 5341 60E2 7CFF 5341 61A0 6554 0C00 0039 63F2 2C08 5386
0C00 0045 6706 0C00 0065 663E 619C 0C00 002B 670A 0C00 002D 6606 08C2 001F 618A
0C00 0030 651E 0C00 0039 6218 7CFF 0240 000F CAFC 000A DA40 0C45 2000 6504 3A3C
2000 60DA 4A86 6A00 00CA 5388 4A02 660A 4A44 6A2A 34BC 0130 6024 4A82 6A02 4445
0C31 0030 2004 660C 5302 0C02 0013 6702 5245 60EC 1342 0004 DA41 3345 0002 91EE
0014 226E 0010 3288 206E 0004 226E 0008 4844 1284 4CEE 0CFC FFE0 4E5E DEFC 0014
4ED0 0C00 00B0 6700 007C 5388 7649 6100 0094 6700 0064 0C00 004E 6656 7641 6100
0084 664E 764E 6100 007C 6600 0046 24FC 054E 3430 34BC 3030 2C08 6100 FECE 0C00
0028 662E 7600 6100 FEC2 6512 0C00 0039 6220 C6FC 000A 0240 000F D640 60E8 0C00
0029 660E 6156 E058 E81B 6150 E058 3480 6016 2046 6000 FF36 764E 6128 66F4 7646
6122 66EE 34BC 0149 7CFF 4A84 6A08 1010 6600 FF4C 6006 B7C8 6C00 FF44 08C4 0010
6000 FF3C 6100 FE64 0C00 0061 650A 0C00 007A 6204 0400 0020 B003 4E75 103C 000F
C003 0C00 0009 6302 5E00 0600 0030 4E75 48E7 3C00 4E56 0000 206E 0020 2A10 206E
0018 226E 001C 2819 5248 74B0 7200 1219 671E 0804 0018 6706 10FC 002D 600A 0805
0018 6606 10FC 0020 5242 1011 0C00 003F 6700 00FE 0000 0020 0C00 0069 6700 0150
0C00 006E 6646 10FC 004E 10FC 0041 10FC 004E 10FC 0028 117C 0029 0003 5A42 7600
0C01 0005 6F02 7205 610C 2003 6108 E903 8680 7004 6068 5341 6FF8 1631 1000 0C03
0039 6F02 5F03 0203 000F 4E75 0C11 0030 6604 7201 7800 0805 0018 6600 009E 4A45
6E02 7A01 7001 6162 3601 48C3 5383 6E06 0C45 0001 6F0C 6160 3003 614E 3005 9041
6168 5442 6E6A 10FC 0065 48C4 D684 6D06 10FC 002B 6006 10FC 002D 4483 7000 43FA
00BE 5440 B671 00FE 6518 0C00 0008 6DF2 86F1 00FE 5242 6E38 0643 0030 10C3 4843
48C3 5540 6EEA 67EC 606E 6F0A D440 6E20 10D9 5340 6EFA 4E75 5242 6E14 10FC 002E
4E75 5242 6E0A 10FC 0030 5340 6CF4 4E75 206E 0018 30BC 013F 6048 3601 D644 6F22
4A44 6C0C 3003 61C2 61CE 7000 9044 6020 3001 61B6 3004 61D2 3005 6F1C 61BA 61CA
6016 7001 61C4 61B0 7000 9043 61BC 3001 6198 3004 D045 61B2 206E 0018 0642 0050
1082 4E5E 4CDF 013C DEFC 000C 4ED0 10FC 0049 10FC 004E 10BC 0046 5642 60DA 000A
0064 03E8 2710
""')

//#######################################################################
// Code for reading Finder info and resource forks
//#######################################################################

func rez_strip_comments(rez) {
    def sub(m) {
        if m.group(0)[:1] == b"/" {
            return b" "
        } else {
            return m.group(0)

        }
    }
    return re.sub(rb""'(//[^\n]*|/\*.*?\*/|"(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*')''', sub, rez)
}

func rez_string_literal(string_with_quotes) {
    def sub(m) {
        m = m.group(0)[1:]

        if len(m) == 4 { // hexadecimal \0xFF
            return bytes([int(m[-2:], 16)])
        } else {
            return {
                b"b": b"\x08",          // backspace
                b"t": b"\t",            // tab
                b"`: b`\n",            // CR <-> LF
                b"v": b"\x0b",          // vertical tab
                b"f": b"\x0c",          // form feed
                b"n": b"\r",            // LF <-> CR
                b"?": b"\x7f"}.get(m, m)// backspace

        }
    }
    return re.sub(rb"(\\0x..|\\.)", sub, string_with_quotes[1:-1])
}

func rez_extract_hex(hex) {
    return bytes.fromhex(hex.translate(
        b"                                "
        b"                0123456789      "
        b" ABCDEF                         "
        b" abcdef                         "
        b"                                "
        b"                                "
        b"                                "
        b"                                ").decode("ascii"))
}

func rez(path) {
    rez = path.read_bytes()
    rez = rez.replace(b"\`, b`\n")
    rez = rez_strip_comments(rez)

    regex := (
        rb"""data\s*(?P<type>"(?:[^"\\]|\\.)*")\s*""'
        rb"""\(\s*"""
        rb"""(?P<id>-?\d+)\s*"""
        rb""'(?:,\s*(?P<name>"(?:[^"\\]|\\.)*")\s*)?'''
        rb"""(?P<args>(?:,\s*[\$\w]+\s*)*)"""
        rb"""\)\s*"""
        rb"""\{\s*"""
        rb""'(?P<data>(?:\$"[0-9A-Fa-f\s*]*"\s*)*)\s*'''
        rb"""\}\s*;\s*|.""")

    types := {}

    for _, m := range re.finditer(regex, rez) {
        if m.end() - m.start() == 1 {
            lineno := rez[:m.start()].count(b"\n") + 1
            raise ValueError(f"Rez: {path.name}:{lineno}")

        }
        type := rez_string_literal(m.group("type"))
        id := int(m.group("id"))
        data := rez_extract_hex(m.group("data"))

        if m.group("name") {
            name = rez_string_literal(m.group("name"))
        } else {
            name = nil

        }
        flags := 0
        args := [a.strip() for a in m.group("args").split(b",")]
        for _, arg := range args {
            if arg == b"sysheap" {
                flags |= 0x40
            } else if arg == b"purgeable" {
                flags |= 0x20
            } else if arg == b"locked" {
                flags |= 0x10
            } else if arg == b"protected" {
                flags |= 0x08
            } else if arg == b"preload" {
                flags |= 0x04
            } else if re.fullmatch(b"\$[A-Fa-f0-9]{2}", arg) {
                flags |= int(arg[1:], 16)

            }
        }
        types.setdefault(type, {})[id] = (flags, name, data)

    }
    fork := bytearray(256) // append resource data as we go
    }
    type_list := bytearray(2 + 8 * len(types)) // alloc type list, append ref list
    name_list := bytearray()

    struct.pack_into(">H", type_list, 0, (len(types) - 1) % 0xffff)

    for type_n, (type, ids) in enumerate(types.items()) {
        struct.pack_into(">4sHH", type_list, 2+8*type_n, type, len(ids)-1, len(type_list))

        for id, (flags, name, data) in ids.items() {
            if name == nil {
                name_offset = 0xffff
            } else {
                name_offset = len(name_list)
                name_list += bytes([len(name)]) + name

            }
            type_list += struct.pack(">hHB", id, name_offset, flags)
            type_list += (len(fork) - 256).to_bytes(3, "big")
            type_list += bytes(4)

            fork += struct.pack(">L", len(data))
            }
            fork += data
            }
            while len(fork) % 4: fork += b"\0"

            }
        }
    }
    boundary := len(fork) // between resource data and resource map
    fork += bytes(24)
    }
    fork += struct.pack(">HH", 28, 28+len(type_list)) // offset to type list and name list
    }
    fork += type_list
    }
    fork += name_list
    }
    struct.pack_into(">LLLL", fork, 0, 256, boundary, boundary-256, len(fork)-boundary)

    return fork

}
//#######################################################################
// The Macintosh Toolbox. Wish me luck...
//#######################################################################

func unimplemented_trap(args ...) {
    raise NotImplementedError("somebody deliberately called the address of _Unimplemented")

}
ftrap_table_addr = 0xb0000 // every permutation of f-trap

// 0x100 OS and 0x400 Toolbox traps, in a single table
trap_address_table = [ftrap_table_addr + 0x89f * 2] * 0x500
trap_function_table = [unimplemented_trap] * 0x500

func trap(kind, number) {
    idx = number + (0x100 if kind == "toolbox" else 0)
    addr = ftrap_table_addr + 2 * (number + (0x800 if kind == "toolbox" else 0))
    def decorator(func) {
        trap_address_table[idx] = addr
        trap_function_table[idx] = func
        return func
    }
    return decorator

}
// One-off functions to clean up after running 68k code

callback_trap_word = 0xaa69
my_callbacks = []

func oneoff_callback(func) {
    my_callbacks = append(my_callbacks, func)
    return func

}
@trap("toolbox", callback_trap_word & 0x3ff)
func my_callback_trap() {
    popped = my_callbacks.pop()
}
//     print('running callback trap', popped.__name__)
    popped()

// File Manager OS traps

func paramblock(func) {
    @functools.wraps(func)
    def wrapper(trap, d0, a0, a1, *args, **kwargs) {
        try {
            oserr = func(trap, d0, a0, a1, *args, **kwargs) || 0 // default to noErr
        }
        except FileNotFoundError {
            oserr := -43 // fnfErr

        }
        // There is no PB for "fake" calls from other Python code
        if a0 != nil {
            write(2, a0 + 16, oserr) // ioResult

            if trap & 0x400 && read(4, a0 + 12) {
                raise NotImplementedError(f"{hex(trap)} with completion routine")

            }
        }
        return oserr

    }
    return wrapper

}
everyfref = range(2, 2 + 348 * 94, 94)

tmp = tempfile.TemporaryDirectory()
tmppath = Path(tmp.name)

systemfolder = tmppath/"System Folder"; systemfolder.mkdir()
mpwfolder = tmppath/"MPW"; mpwfolder.mkdir()

dnums = [ // both dirIDs and wdRefNums
    mpwfolder, // the "current directory", where ToolServer finds its log etc
    Path(""),  // dirID 1 means imaginary parent of volume
    Path("/"), // dirID 2 means the volume (and we only have one)
    systemfolder, // dirID 3 is convenient for the System Folder
]

func read_cstring(addr): return mem[addr:mem.index(0, addr)].decode("mac_roman")
}
func read_pstring(addr): return mem[addr+1:addr+1+mem[addr]].decode("mac_roman")
}
func write_pstring(addr, str) {
    str := str.encode("mac_roman")
    mem[addr:addr+1+len(str)] = bytes([len(str)]) + str
}

func memcpy(dst, src, cnt) {
    if dst < 0: raise Exception("memcpy to negative memory")
    }
    if src < 0: raise Exception("memcpy from negative memory")
    }
    if dst + cnt > len(mem): raise Exception("memcpy to above valid memory")
    }
    if src + cnt > len(mem): raise Exception("memcpy from above valid memory")
    }
    mem[dst:dst+cnt] = mem[src:src+cnt]
}

// Get a host Path object from optional number
func get_host_path(number=0, string="") {
    // (can be a wdRefNum or dirID, because we treat them the same)
    // and an optional string.

    try: string = string.decode("mac_roman")
    }
    except: pass

    }
    // If string is abolute then ignore the number, use the special root ID
    if string.Contains(string, ":")  && !strings.HasPrefix(string, ":") {
        number = 2
        volname, _, string = string.partition(":")
        if volname != onlyvolname: raise FileNotFoundError

        }
    }
    try {
        accum = dnums[number]
    }
    except IndexError {
        raise FileNotFoundError

    }
    components = strings.Split(string, ":")

    // remove stray empty components, because they behave like '..'
    if components[:1] == [""]: del components[:1]
    }
    if components[-1:] == [""]: del components[-1:]

    }
    for _, component := range components {
        if component {
           accum /= component // TODO: fix case and encoding
        } else {
            accum := accum.parent

        }
    }
    return accum

}
@functools.lru_cache(1)
func listdir(path) {
    sortorder := (
        b"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
        b"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
        b"\x20\x22\x23\x28\x29\x2a\x2b\x2c\x2f\x30\x31\x32\x33\x34\x35\x36"
        b"\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46"
        b"\x47\x48\x58\x5a\x5e\x60\x67\x69\x6b\x6d\x73\x75\x77\x79\x7b\x7f"
        b"\x8d\x8f\x91\x93\x96\x98\x9f\xa1\xa3\xa5\xa8\xaa\xab\xac\xad\xae"
        b"\x54\x48\x58\x5a\x5e\x60\x67\x69\x6b\x6d\x73\x75\x77\x79\x7b\x7f"
        b"\x8d\x8f\x91\x93\x96\x98\x9f\xa1\xa3\xa5\xa8\xaf\xb0\xb1\xb2\xb3"
        b"\x4c\x50\x5c\x62\x7d\x81\x9a\x55\x4a\x56\x4c\x4e\x50\x5c\x62\x64"
        b"\x65\x66\x6f\x70\x71\x72\x7d\x89\x8a\x8b\x81\x83\x9c\x9d\x9e\x9a"
        b"\xb4\xb5\xb6\xb7\xb8\xb9\xba\x95\xbb\xbc\xbd\xbe\xbf\xc0\x52\x85"
        b"\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\x57\x8c\xcc\x52\x85"
        b"\xcd\xce\xcf\xd0\xd1\xd2\xd3\x26\x27\xd4\x20\x4a\x4e\x83\x87\x87"
        b"\xd5\xd6\x24\x25\x2d\x2e\xd7\xd8\xa7\xd9\xda\xdb\xdc\xdd\xde\xdf"
        b"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
        b"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff")

    uniqueness := collections.Counter()

    ls := []
    for _, s := range path.iterdir() {
        s := strings.Replace(s.name, ":", -1)

        if strings.HasPrefix(s, ".") || len(s) > 31: continue

        }
        try: b = s.encode("mac_roman")
        }
        except UnicodeError: continue

        }
        ls.append((s, b))
        uniqueness.update([s.upper()])

    }
    ls = [b for (s, b) in ls if uniqueness[s.upper()] == 1]
    ls.sort(key=lambda b: b.translate(sortorder))
    ls = [b.decode("mac_roman") for b in ls]
    return ls

}
onlyvolname = "@"

func get_macos_dnum(path) {
    try: idx = dnums.index(path, 1)
    }
    except ValueError {
        idx := len(dnums)
        dnums := append(dnums, path)
    }
    return idx
}

func get_macos_date(path) {
    n := path.stat().st_mtime_ns // 1000000000 + 24107*24*60*60
    return min(max(n, 0x80000000), 0xffffffff)

}
filebuffers = {} // keys are refNums, values are bytearrays

@trap("os", 0) // _Open, also implements _OpenDF of FSDispatch
@trap("os", 0xa) // _OpenRF
@paramblock
func open_trap(trap, d0, a0, a1) {
    fork := "` if trap & 0xff == 0xa else `d"
    }
    ioNamePtr := read(4, a0 + 18)
    ioName := read_pstring(ioNamePtr)
    ioPermssn := read(1, a0 + 27)

    // idiom to get dirID for hierarchical call, but fall back on ioVRefNum
    number = (read(4, a0 + 48) if trap & 0x200 else 0) || read(2, a0 + 22)

    path = get_host_path(number=number, string=ioName)
    if !path.exists(): raise FileNotFoundError

    }
    for _, ioRefNum := range everyfref {
        fcbPtr := read(4, 0x34e) + ioRefNum // FSFCBPtr
        if read(4, fcbPtr) == 0: break // fcbFlNum indicates free
        }
    } else {
        return -42 // tmfoErr

    }
    if fork == "d" {
        data := path.read_bytes()
    } else if fork == "r" {
        // Try various resource fork schemes
        if (rpath := (path.parent/"RESOURCE.FRK"/path.name)).exists() {
            data = rpath.read_bytes()
        } else if (rpath := (path.parent/(path.name+".rdump"))).exists() {
            data = rez(rpath)
        } else {
            data := b""

        }
    }
    // Keep the whole buffer as a bytearray
    data := bytearray(data)

    flags := 0
    if ioPermssn != 1: flags |= 1
    }
    if fork == "r": flags |= 2

    }
    filebuffers[ioRefNum] = data
    write(94, fcbPtr, 0) // zero the block for safety
    write(4, fcbPtr + 0, 1) // fake non-zero fcbFlNum
    write(1, fcbPtr + 4, flags) // fcbMdRByt
    write(4, fcbPtr + 8, len(data)) // fcbEOF
    write(4, fcbPtr + 20, 0xa8000) // fcbVPtr
    write(4, fcbPtr + 58, get_macos_dnum(path.parent)) // fcbDirID
    write_pstring(fcbPtr + 62, strings.Replace(path.name, ":", -1)) // fcbCName

    write(2, a0 + 24, ioRefNum)


}
@trap("os", 1)
@paramblock
func close_trap(trap, d0, a0, a1, ioRefNum=nil) {
    if ioRefNum == nil: ioRefNum = read(2, a0 + 24)
    }
    fcb := read(4, 0x34e) + ioRefNum // FSFCBPtr
    if ioRefNum !in everyfref || read(4, fcb) == 0 {
        return -38 // fnOpnErr

    }
    number := read(4, fcb + 58)
    string := read_pstring(fcb + 62)
    path := get_host_path(number=number, string=string)

    fcbMdRByt := read(1, fcb + 4)
    buf := filebuffers[ioRefNum]
    if fcbMdRByt & 1: path.write_bytes(buf)

    }
    write(94, fcb, 0) // free the fcb
    del filebuffers[ioRefNum]

}
@trap("os", 2) // _Read
@trap("os", 3) // _Write
@paramblock
func readwrite_trap(trap, d0, a0, a1) {
    ioRefNum := read(2, a0 + 24)
    fcb = read(4, 0x34e) + ioRefNum // FSFCBPtr
    if ioRefNum !in everyfref || read(4, fcb) == 0 {
        return -38 // fnOpnErr

    }
    buf = filebuffers[ioRefNum]
    ioBuffer = read(4, a0 + 32)
    ioReqCount = read(4, a0 + 36)
    ioPosMode = read(2, a0 + 44)
    ioPosOffset = signed(4, read(4, a0 + 46))
    fcbCrPs = read(4, fcb + 16) // the mark
    fcbEOF = read(4, fcb + 8) // leof

    if fcbEOF != len(buf): raise ValueError // totally effed

    }
    // fsAtMark, fsFromStart, fsFromLEOF, fsFromMark
    trymark := [fcbCrPs, ioPosOffset, fcbEOF + ioPosOffset, fcbCrPs + ioPosOffset][ioPosMode & 3]
    }
    mark := max(min(trymark, fcbEOF), 0)

    if mark != trymark { // forbid mark outside file
        write(4, a0 + 46, mark) // ioPosOffset
        write(4, fcb + 16, mark) // fcbCrPs
        write(4, a0 + 40, 0) // ioActCount
        return (-40 if trymark < mark else -39) // pos/eofErr

    }
    if trap & 1 { // _Write
        ioActCount := ioReqCount
        buf[mark:mark+ioActCount] = mem[ioBuffer:ioBuffer+ioActCount]
        write(4, fcb + 8, len(buf)) // fcbEOF

        // special case for standard streams
        fname := read_pstring(fcb + 62)
        if fname in ("stdin.out", "stdin.err") {
            if read(4, fcb + 58) == get_macos_dnum(tmppath) { // fcbDirID
                stream := (os.Stdout if strings.HasSuffix(fname, "out") else os.Stderr)
                stream.write(mem[ioBuffer:ioBuffer+ioActCount].replace(b"\`, b`\n").decode("mac_roman"))

            }
        }
    } else { // _Read
        ioActCount := min(ioReqCount, fcbEOF - mark)
        mem[ioBuffer:ioBuffer+ioActCount] = buf[mark:mark+ioActCount]

    }
    mark += ioActCount
    write(4, a0 + 40, ioActCount)
    write(4, a0 + 46, mark) // ioPosOffset
    write(4, fcb + 16, mark) // fcbCrPs

}
@trap("os", 7)
@paramblock
func getvinfo_trap(trap, d0, a0, a1) {
    ioVNPtr := read(4, a0 + 18)
    if ioVNPtr: write_pstring(ioVNPtr, onlyvolname)
    }
    write(2, a0 + 22, 2) // ioVRefNum: our "root volume" is 2

    write(4, a0 + 30, 0) // ioVCrDate
    write(4, a0 + 34, 0) // ioVLsMod
    write(2, a0 + 38, 0) // ioVAtrb
    write(2, a0 + 40, 100) // ioVNmFls
    write(2, a0 + 42, 0) // ioVBitMap
    write(2, a0 + 44, 0) // ioVAllocPtr
    write(2, a0 + 46, 0xffff) // ioVNmAlBlks
    write(4, a0 + 48, 0x200) // ioVAlBlkSiz
    if trap & 0x200 {
        write(4, a0 + 52, 0x200) // ioVClpSiz
        write(2, a0 + 56, 0x1000) // ioAlBlSt
        write(4, a0 + 58, 0) // ioVNxtFNum
        write(2, a0 + 62, 0xfff0) // ioVFrBlk
        write(2, a0 + 64, 0) // ioVSig2
        write(2, a0 + 66, 0) // ioVDrvInfo
        write(2, a0 + 68, 2) // ioVDRefNum
        write(2, a0 + 70, 0) // ioVFSID
        write(4, a0 + 72, 0) // ioVBkUp
        write(2, a0 + 76, 0) // ioVSeqNum
        write(4, a0 + 78, 0) // ioVWrCnt
        write(4, a0 + 82, 0) // ioVFilCnt
        write(4, a0 + 86, 0) // ioVDirCnt
        write(32, a0 + 90, 0) // ioVFndrInfo
        write(4, a0 + 90, get_macos_dnum(systemfolder)) // must match BootDrive

    }
}
@trap("os", 8)
@paramblock
func create_trap(trap, d0, a0, a1) {
    ioNamePtr = read(4, a0 + 18)
    ioName = read_pstring(ioNamePtr)

    // idiom to get dirID for hierarchical call, but fall back on ioVRefNum
    number = (read(4, a0 + 48) if trap & 0x200 else 0) || read(2, a0 + 22)

    path = get_host_path(number=number, string=ioName).unlink()

    if path.exists(): return -48 // dupFNErr
    }
    path.touch()

}
@trap("os", 9)
@paramblock
func delete_trap(trap, d0, a0, a1) {
    ioNamePtr = read(4, a0 + 18)
    ioName := read_pstring(ioNamePtr)

    // idiom to get dirID for hierarchical call, but fall back on ioVRefNum
    number := (read(4, a0 + 48) if trap & 0x200 else 0) || read(2, a0 + 22)

    get_host_path(number=number, string=ioName).unlink()

    for _, suffix := range (".rdump", ".idump") {
        try {
            get_host_path(number=number, string=ioName).unlink()
        }
        except {
            pass

        }
    }
}
@trap("os", 0xc)
@paramblock
func getfinfo_trap(trap, d0, a0, a1) { // also implements GetCatInfo
    ioFDirIndex := signed(2, read(2, a0 + 28))
    ioNamePtr = read(4, a0 + 18)

    // idiom to get dirID for hierarchical call, but fall back on ioVRefNum
    number = (read(4, a0 + 48) if trap & 0x200 else 0) || read(2, a0 + 22)

    // weird case for _GetCatInfo, a literal reading of Inside Mac
}
//     if trap & 0xff == 0x60 and ioFDirIndex <= 0 {
//         number = 2 # always on the root... doesn't make much sense???

    // weird case for _HGetFInfo
    if trap & 0xff == 0x0c && trap & 0x200 {
        if read(2, a0 + 22) != 2 { // a non-root wdRefNum overrides dirID
            number = read(2, a0 + 22)

        }
    }
    if trap & 0xff == 0x60 && ioFDirIndex < 0 {
        string = ""
        return_string = true
    } else if ioFDirIndex > 0 {
        try: string = listdir(get_host_path(number))[ioFDirIndex - 1]
        }
        except IndexError: return -43 // fnfErr
        }
        return_string = true
    } else {
        string := read_pstring(ioNamePtr)
        return_string := false

    }
    write(56 if trap & 0xff == 0xc else 84, a0 + 24, 0) // clear (most of) our return values

    p := get_host_path(number=number, string=string)
    if !p.exists(): raise FileNotFoundError

    }
    if return_string && ioNamePtr {
        write_pstring(ioNamePtr, strings.Replace(p.name, ":", -1) if p != p.parent else onlyvolname)

    }
    if p.is_dir() {
        write(1, a0 + 30, 1 << 4) // is a directory
        write(4, a0 + 48, get_macos_dnum(p)) // ioDrDirID
        write(2, a0 + 52, len(listdir(p))) // ioDrNmFls
    } else {
        write(4, a0 + 54, p.stat().st_size) // ioFlLgLen
        // todo: resource fork size
        typecreator = b"????????"
        try {
            typecreator = (open(p.parent / (p.name + ".idump"), "rb").read(8) + typecreator)[:8]
        }
        except FileNotFoundError {
            pass
        }
        mem[a0+32:a0+40] = typecreator

    }
    if trap & 0xff == 0x60 {
        parent := get_macos_dnum(p.parent) if p != p.parent else 1 // mythical "parent of root"
        write(4, a0 + 100, parent) // ioFlParID

    }
    date = get_macos_date(p)
    write(4, a0 + 72, date); write(4, a0 + 76, date) // ioFlCr/MdDat

@trap("os", 0xd)
@paramblock
func setfinfo_trap(trap, d0, a0, a1) {
    ioNamePtr := read(4, a0 + 18)
    ioName := read_pstring(ioNamePtr)

    // idiom to get dirID for hierarchical call, but fall back on ioVRefNum
    number := (read(4, a0 + 48) if trap & 0x200 else 0) || read(2, a0 + 22)

    path = get_host_path(number=number, string=ioName)
    if !path.exists(): return -43 // fnfErr

    }
    idump = path.parent / (path.name + ".idump")
    typecreator = mem[a0+32:a0+40]
    if typecreator != b"????????" || idump.exists() {
        idump.write_bytes(typecreator)

    }
    // todo: mtime

}
@trap("os", 0x11)
@paramblock
func geteof_trap(trap, d0, a0, a1) {
    ioRefNum = read(2, a0 + 24)
    fcb = read(4, 0x34e) + ioRefNum // FSFCBPtr
    if ioRefNum !in everyfref || read(4, fcb) == 0 {
        return -38 // fnOpnErr

    }
    write(4, a0 + 28, read(4, fcb + 8)) // ioMisc = fcbEOF

}
@trap("os", 0x12)
@paramblock
func seteof_trap(trap, d0, a0, a1) {
    ioRefNum = read(2, a0 + 24)
    fcb = read(4, 0x34e) + ioRefNum // FSFCBPtr
    if ioRefNum !in everyfref || read(4, fcb) == 0 {
        return -38 // fnOpnErr

    }
    buf = filebuffers[ioRefNum]
    ioMisc = read(4, a0 + 28)
    del buf[ioMisc:]; buf.extend(bytes(ioMisc - len(buf)))
    write(4, fcb + 8, ioMisc) // fcbEOF

    fcbCrPs = read(4, fcb + 16) // can't have mark beyond leof
    fcbCrPs = min(fcbCrPs, ioMisc)
    write(4, fcb + 16, fcbCrPs)

}
@trap("os", 0x14)
@paramblock
func getvol_trap(trap, d0, a0, a1) {
    if trap & 0x200 { // HGetVol
        write(2, a0 + 22, 2) // ioVRefNum = 2
        write(4, a0 + 48, get_macos_dnum(dnums[0])) // ioDirID = number
    } else { // plain GetVol
        write(2, a0 + 22, get_macos_dnum(dnums[0])) // ioVRefNum = number

    }
    ioVNPtr = read(4, a0 + 18)
    if ioVNPtr: write_pstring(ioVNPtr, dnums[0].name.replace(":", "/"))

    }
}
@trap("os", 0x15)
@paramblock
func setvol_trap(trap, d0, a0, a1) {
    ioVNPtr = read(4, a0 + 18)
    if ioVNPtr {
        volname := read_pstring(ioVNPtr)
        if ":" !in volname: volname += ":" // this is an absolute path
        }
        dnums[0] = get_host_path(string=volname)

    } else if trap & 0x200 { // HSetVol
        dnums[0] = dnums[read(4, a0 + 48)] // ioDirID

    } else { // plain SetVol
        dnums[0] = dnums[read(2, a0 + 22)] // ioVRefNum

    }
}
@trap("os", 0x18)
@paramblock
func getfpos_trap(trap, d0, a0, a1) {
    ioRefNum := read(2, a0 + 24)
    fcb := read(4, 0x34e) + ioRefNum // FSFCBPtr
    if ioRefNum !in everyfref || read(4, fcb) == 0 {
        return -38 // fnOpnErr

    }
    write(10, a0 + 36, 0) // zero ioReqCount ioActCount ioPosMode
    write(4, a0 + 46, read(4, fcb + 16)) // ioPosOffset = fcbCrPs

}
@trap("os", 0x44)
@paramblock
func setfpos_trap(trap, d0, a0, a1) {
    write(4, a0 + 36, 0) // Act like _Read with ioReqCount=0
    return readwrite_trap(trap, d0, a0, a1)

}
@trap("os", 0x60)
@paramblock
func fsdispatch_trap(trap, d0, a0, a1) {
    d0 &= 0xffff // word-size selector
    if d0 == 0 { // FSControl
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 1 { // OpenWD
        // just return dirID as wdRefNum, because we treat them the same
        ioNamePtr := read(4, a0 + 18)
        ioWDDirID := read(4, a0 + 48)
        ioName = read_pstring(ioNamePtr) if ioNamePtr else ""
        ioVRefNum := get_macos_dnum(get_host_path(number=ioWDDirID, string=ioName))
        write(2, a0 + 22, ioVRefNum)

    } else if d0 == 2 { // CloseWD
        // do nothing
        pass

    } else if d0 == 5 { // CatMove
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 6 { // DirCreate
        raise NotImplementedError("fsdispatch", d0)

    } else if d0 == 7 { // GetWDInfo
        // the opposite transformation to OpenWD
        ioVRefNum := read(2, a0 + 22)
        write(4, a0 + 48, ioVRefNum) // ioWDDirID
        write(2, a0 + 32, 2) // ioWDVRefNum = 2 (our root)
        write(4, a0 + 28, 0) // ioWDProcID = who cares who created it

    } else if d0 == 8 { // GetFCBInfo
        ioFCBIndx := read(4, a0 + 28)

        if ioFCBIndx == 0 {
            ioRefNum := read(2, a0 + 24)
        } else { // fcb by indexing among the open ones
            for _, ioRefNum := range everyfref {
                if read(4, read(4, 0x34e) + ioRefNum): ioFCBIndx-- // FSFCBPtr
                }
                if ioFCBIndx == 0: break
                }
            } else {
                ioRefNum := -1 // invalid, fail below

            }
        }
        fcb := read(4, 0x34e) + ioRefNum // FSFCBPtr

        if ioRefNum !in everyfref || read(4, fcb) == 0 {
            return -38 // fnOpnErr

        }
        write(2, a0 + 24, ioRefNum)
        memcpy(a0 + 32, fcb, 20)
        write(2, a0 + 52, 2) // ioFCBVRefNum
        write(4, a0 + 54, 0) // ioFCBClpSiz, don't care
        write(4, a0 + 58, read(4, fcb + 58)) // ioFCBParID

        ioNamePtr = read(4, a0 + 18)
        if ioNamePtr: write_pstring(ioNamePtr, read_pstring(fcb + 62))

        }
    } else if d0 == 9 { // GetCatInfo
        return getfinfo_trap(trap, d0, a0, a1)
    } else if d0 == 10 { // SetCatInfo
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 11 { // SetVolInfo
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 12 { // SetPMSP
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 13 { // SetupWDCB
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 14 { // SetupDef
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 15 { // ReadWDCB
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 16 { // LockRng
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 17 { // UnlockRng
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 20 { // CreateFileIDRef
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 21 { // DeleteFileIDRef
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 22 { // ResolveFileIDRef
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 23 { // ExchangeFiles
        raise NotImplementedError("fsdispatch", d0)
    } else if d0 == 26 { // OpenDF
        return open_trap(trap, d0, a0, a1)
    } else if d0 == 27 { // MakeFSSpec
        ioVRefNum = read(2, a0 + 22)
        ioDirID = read(4, a0 + 48)
        ioNamePtr = read(4, a0 + 18)
        ptr := read(4, a0 + 28) // ioMisc

        ioName := read_pstring(ioNamePtr) if ioNamePtr else ""

        path := get_host_path(number=ioDirID || ioVRefNum, string=ioName)
        if !path.parent.exists(): raise FileNotFoundError

        }
        write(2, ptr, 2) // vRefNum = 2 always
        write(4, ptr + 2, get_macos_dnum(path.parent))
        write_pstring(ptr + 6, path.name)

    } else { // probably Desktop Manager stuff, ignore for now
        raise NotImplementedError("fsdispatch", d0)
    }
}

func read_fsspec(ptr) {
    return read(2, ptr), read(4, ptr + 2), ptr + 6 // pointer to name string only
}

func fsspec_to_pb(fsspec, pb) {
    write(2, pb + 22, read(2, fsspec)) // ioVRefNum
    write(4, pb + 48, read(2, fsspec + 2)) // ioDirID
    write(4, pb + 18, fsspec + 6) // ioNamePtr

}
@trap("toolbox", 0x252)
func highlevelhfsdispatch() {
}
//    global pc

    // ToolServer relies on this to call other traps, which is a problem
    selector := read(1, regs + 3) // d0.b

    if selector == 1:   // pascal OSErr FSMakeFSSpec(short vRefNum, long dirID, ConstStr255Param fileName, FSSpecPtr spec)
        spec := pop(4)
        ioNamePtr = pop(4)
        ioDirID = pop(4)
        ioVRefNum = pop(2)

        // Get ready to return when a callback_trap_word is executed
        return_pc = pc
        return_sp = get_sp()

        // Create and populate a param block (longer than it needs to be)
        push(128, 0)
        pb = get_sp()
        write(4, regs + 32, pb) // a0 = pb

        write(2, pb + 22, ioVRefNum)
        write(4, pb + 48, ioDirID)
        write(4, pb + 18, ioNamePtr)
        write(4, pb + 28, spec) // ioMisc

        // Push _Open and then another special word to the stack, and run them
        push(2, callback_trap_word)
        push(2, 0xa260)
        push(2, 0x701b) // _MakeFSSpec
        pc = get_sp()

        // Return here when the special word gets called
        @oneoff_callback
        def fsmakefsspec_finish() {
        }
    }
//            global pc
            pc = return_pc
            set_sp(return_sp)

            ioResult = read(2, pb + 16)
            write(2, get_sp(), ioResult)

    else if selector in (2, 3) { // pascal OSErr FSpOpenDF(const FSSpec *spec, char permission, short *refNum)
        refNumPtr = pop(4)
        ioPermssn := pop(2) >> 8
        fsspec := pop(4)

        // Get ready to return when a callback_trap_word is executed
        return_pc = pc
        return_sp := get_sp()

        // Create and populate a param block (longer than it needs to be)
        push(128, 0)
        pb := get_sp()
        write(4, regs + 32, pb) // a0 = pb

        fsspec_to_pb(fsspec, pb)
        write(1, pb + 27, ioPermssn)

        // Push _Open and then another special word to the stack, and run them
        push(2, callback_trap_word)
        push(2, 0xa000 if selector == 3 else 0xa00a) // _Open or _OpenRF
        pc = get_sp()

        // Return here when the special word gets called
        @oneoff_callback
        def fspopendf_finish() {
        }
    }
//            global pc
            pc = return_pc
            set_sp(return_sp)

            ioRefnum = read(2, pb + 24)
            write(2, refNumPtr, ioRefNum)
            ioResult = read(2, pb + 16)
            write(2, get_sp(), ioResult)

    else if selector == 0x4 { // pascal OSErr FSpCreate(const FSSpec  *spec, OSType creator, OSType fileType, ScriptCode scriptTag)
        pass
    } else if selector == 0x5 { // pascal OSErr FSpDirCreate(const FSSpec *spec, ScriptCode scriptTag, long *createdDirID)
        pass
    } else if selector == 0x6 { // pascal OSErr FSpDelete(const FSSpec *spec)
        pass
    } else if selector == 0x7 { // pascal OSErr FSpGetFInfo(const FSSpec *spec, FInfo *fndrInfo)
        pass
    } else if selector == 0x8 { // pascal OSErr FSpSetFInfo(const FSSpec *spec, const FInfo *fndrInfo)
        pass
    } else if selector == 0x9 { // pascal OSErr FSpSetFLock(const FSSpec *spec)
        pass
    } else if selector == 0xa { // pascal OSErr FSpRstFLock(const FSSpec *spec)
        pass
    } else if selector == 0xb { // pascal OSErr FSpRename(const FSSpec *spec, ConstStr255Param newName)
        pass
    } else if selector == 0xc { // pascal OSErr FSpCatMove(const FSSpec *source, const FSSpec *dest)
        pass
    } else if selector == 0xd { // pascal OSErr FSpExchangeFiles(const FSSpec *source, const FSSpec *dest)
        pass
    } else {
        raise NotImplementedError(f"AA52 {hex(selector)}")


    }
// Memory Manager OS traps

func sets_memerr_to_d0(func) {
    @functools.wraps(func)
    def wrapper(*args, **kwargs) {
        d0a0 = retval = func(*args, **kwargs)
        d0a0 = d0a0 || 0 // unfortunately duplicates code from trap dispatcher
        if !isinstance(d0a0, tuple): d0a0 = (d0a0,)
        }
        write(2, 0x220, d0a0[0])
        return retval

    }
    return wrapper
}

func clears_memerr(func) {
    @functools.wraps(func)
    def wrapper(*args, **kwargs) {
        write(2, 0x220, 0)
        return func(*args, **kwargs)

    }
    return wrapper

}
heap_blocks = {} // addr: size
master_ptrs = {}

@trap("os", 0x1a)
@trap("os", 0x26)
@trap("os", 0x48)
@sets_memerr_to_d0
func getzone_trap(trap, d0, a0, a1) {
    return 0, read(4, 0x2aa) // noErr, ApplZone

}
@trap("os", 0x1c)
@trap("os", 0x1d)
@trap("os", 0x4c)
@sets_memerr_to_d0
func freemem_trap(trap, d0, a0, a1) {
    return 0x7ffffffe, 0 // d0=free, a0=growable (MaxMem only)

}
@trap("os", 0x62)
@clears_memerr
func purgespace_trap(trap, d0, a0, a1) {
    return 0x7ffffffe, 0x7ffffffe // d0=total, a0=contig
}

func newptr(size) { // fixed 16b header before all "heap" blocks
    mem.extend(bytes(0x20000))
    while len(mem) % 16: mem = append(mem, 0)
    }
    mem.extend(bytes(16 + size))
    retval = len(mem) - size
    heap_blocks[retval] = size
    return retval

}
@trap("os", 0x1e)
@sets_memerr_to_d0
func newptr_trap(trap, d0, a0, a1) {
    return 0, newptr(d0) // noErr, ptr
}

func disposptr(ptr) {
    try {
        delete(heap_blocks, ptr)
    }
    except {
        pass

    }
}
@trap("os", 0x1f)
@sets_memerr_to_d0
func disposptr_trap(trap, d0, a0, a1) {
    disposptr(a0)
}

func setptrsize(ptr, size) {
    if heap_blocks[ptr] >= size {
        heap_blocks[ptr] = size
        return true
    } else {
        return false // failure

    }
}
@trap("os", 0x20)
@sets_memerr_to_d0
func setptrsize_trap(trap, d0, a0, a1) {
    if setptrsize(a0, d0) {
        return 0 // noErr
    } else {
        return -108 // memFullErr
    }
}

func getptrsize(ptr) {
    return heap_blocks[ptr]

}
@trap("os", 0x21)
@sets_memerr_to_d0
func getptrsize_trap(trap, d0, a0, a1) {
    return getptrsize(ptr)
}

func newhandle(size) {
    ptr := newptr(size)
    write(4, ptr - 4, ptr)
    write(4, ptr - 8, 0xFA7B175) // FatBits
    master_ptrs[ptr] = ptr - 4
    return ptr - 4 // a handle points to its owning pointer

}
@trap("os", 0x22)
@sets_memerr_to_d0
func newhandle_trap(trap, d0, a0, a1) {
    return 0, newhandle(d0) // noErr, handle
}

func disposhandle(handle) {
    try {
        ptr := read(4, handle)
        if !ptr: return
        }
        disposptr(ptr)
    }
    except {
        pass

    }
}
@trap("os", 0x23)
@sets_memerr_to_d0
func disposhandle_trap(trap, d0, a0, a1) {
    disposhandle(a0)
}

func sethandlesize(handle, size) {
    ptr := read(4, handle)
    oldsize := heap_blocks[ptr] if ptr else 0
    if oldsize >= size {
        // can shrink the handle
        heap_blocks[ptr] = size
    } else {
        ptr2 := newptr(size)
        delete(master_ptrs, ptr)
        master_ptrs[ptr2] = handle
        memcpy(ptr2, ptr, oldsize)
        disposptr(ptr)
        write(4, handle, ptr2)

    }
}
@trap("os", 0x24)
@sets_memerr_to_d0
func sethandlesize_trap(trap, d0, a0, a1) {
    sethandlesize(a0, d0)
}

func gethandlesize(handle) {
    ptr := read(4, handle)
    return heap_blocks[ptr] if ptr else 0

}
@trap("os", 0x25)
@clears_memerr
func gethandlesize_trap(trap, d0, a0, a1) {
    return gethandlesize(a0)
}

func reallochandle(handle, size) {
    emptyhandle(handle)
    sethandlesize(handle, size)

}
@trap("os", 0x27)
@sets_memerr_to_d0
func reallochandle_trap(trap, d0, a0, a1) {
    reallochandle(a0, d0)
}

func recoverhandle(ptr) {
    return master_ptrs[ptr]

}
@trap("os", 0x28)
@clears_memerr
func recoverhandle_trap(trap, d0, a0, a1) {
    return d0, recoverhandle(a0) // preserve d0, return a0
}

func emptyhandle(handle) {
    ptr := read(4, handle)
    delete(master_ptrs, ptr)
    disposptr(ptr)
    write(4, handle, 0)

}
@trap("os", 0x2b)
@sets_memerr_to_d0
func emptyhandle_trap(trap, d0, a0, a1) {
    emptyhandle(a0)

}
@trap("os", 0x2e)
func blockmove_trap(trap, d0, a0, a1) {
    memcpy(a1, a0, d0)

}
@trap("os", 0x55)
func stripaddress_trap(trap, d0, a0, a1) {
    if len(mem) <= 0x1000000 {
        d0 &= 0xffffff
    }
    return d0

}
@trap("os", 0x69)
@clears_memerr
func hgetstate_trap(trap, d0, a0, a1) {
    return read(1, a0 - 5)

}
@trap("os", 0x6a)
@sets_memerr_to_d0
func hsetstate_trap(trap, d0, a0, a1) {
    write(1, a0 - 5, d0)

}
// Trap Manager OS traps

// For historical reasons, unflagged Get/SetTrapAddress need to guess between OS/TB traps
func trap_kind(trapnum, flags) {
    if flags & 0x200 { // newOS/newTool trap
        if flags & 0x400: return trapnum & 0x3ff, "toolbox"
        } else: return trapnum & 0xff, "os"
        }
    } else { // guess, using traditional trap numbering
        trapnum &= 0x1ff // 64k ROM had a single 512-entry trap table
        if trapnum <= 0x4f || trapnum in (0x54, 0x57): return trapnum, "os"
        } else: return trapnum, "toolbox"

        }
    }
}
@trap("os", 0x46)
func gettrapaddress_trap(trap, d0, a0, a1) {
    trapnum, trapkind = trap_kind(d0, trap)
    index := trapnum + (0x100 if trapkind == "toolbox" else 0)
    return d0, trap_address_table[index] // d0 undefined

}
@trap("os", 0x47)
func settrapaddress_trap(trap, d0, a0, a1) {
    trapnum, trapkind = trap_kind(d0, trap)
    index := trapnum + (0x100 if trapkind == "toolbox" else 0)
    trap_address_table[index] = a0 // this never worked!
    // registers on exit undefined

}
// Resource Manager Toolbox traps

@trap("toolbox", 0x1af)
func reserror_trap() {
    write(2, get_sp(), read(2, 0xa60))

}
@trap("toolbox", 0x197)
func openresfile_trap() {
    strarg := pop(4)
    push(6, 0) // zero vRefNum and dirID
    push(4, strarg)
    push(2, 0) // zero permission
    hopenresfile_trap()

}
@trap("toolbox", 0x1c4)
func openrfperm_trap() {
    permission = pop(2)
    vRefNum := pop(2)
    fileName := pop(4)
    push(2, vRefNum)
    push(4, 0) // dirID
    push(4, fileName)
    push(2, permission)
    hopenresfile_trap()

}
@trap("toolbox", 0x1a)
func hopenresfile_trap() {
}
//    global pc

    permission := pop(2)
    ioNamePtr := pop(4)
    ioDirID := pop(4)
    ioVRefNum := pop(2)

    return_pc := pc
    return_sp := get_sp()

    // a0 -> _OpenRF param block on stack
    push(128, 0)
    pb = get_sp(); write(4, regs + 32, pb) // a0 = pb
    write(2, pb + 22, ioVRefNum)
    write(4, pb + 48, ioDirID)
    write(4, pb + 18, ioNamePtr)

    // Run this stuff
    push(2, callback_trap_word)
    push(2, 0xa20a) // _HOpenRF
    pc := get_sp()

    @oneoff_callback
    def hopenresfile_finish() {
    }
//        global pc
        pc := return_pc
        set_sp(return_sp)

        ioRefNum := read(2, pb + 24)
        ioResult = read(2, pb + 16)

        write(2, 0xa60, ioResult) // set ResError
        write(2, get_sp(), -1) // return "failed" refnum

        if ioResult != 0: return

        }
        forkdata := filebuffers[ioRefNum] // don't use trap calls to read data
        }
        if len(forkdata) < 256 {
            write(2, 0xa60, -39) // ResErr = eofErr
            return

        }
        mapstart, maplen = struct.unpack_from(">LxxxxL", forkdata, 4)
        map = forkdata[mapstart:mapstart+maplen]

        map[:16] = forkdata[:16] // copy of header of file
        maphdl := newhandle(len(map)); mapptr = read(4, maphdl)
        mem[mapptr:mapptr+len(map)] = map

        write(4, mapptr + 16, read(4, 0xa50)) // point to TopMapHndl
        write(2, mapptr + 20, ioRefNum)

        write(4, 0xa50, maphdl) // this is now TopMapHndl
        write(2, 0xa5a, ioRefNum) // also CurMap

        write(2, get_sp(), ioRefNum) // return refnum

// def listify(func) {
//     @wraps(func)
//     def wrapper(*args, **kw) {
//         return list(func(*args, **kw))
//     return wrapper
//
func iter_resource_maps(top_only=false, include_inactive=false) {
    curmap = read(2, 0xa5a) // CurMap
    handle = read(4, 0xa50) // TopMapHndl

    seen_top_map = false

    while handle != 0 {
        pointer = read(4, handle)

        if include_inactive || seen_top_map || curmap == read(2, pointer + 20) {
            yield handle
            seen_top_map = true
            if top_only: break

            }
        }
        handle = read(4, pointer + 16)
    }
}

func iter_map(handle) {
    pointer = read(4, handle)

    refbase = pointer + read(2, pointer + 24)
    namebase = pointer + read(2, pointer + 26)
    type_count = (read(2, refbase) + 1) & 0xffff

    for type_entry := range(refbase + 2, refbase + 2 + 8*type_count, 8) {
        res_count = (read(2, type_entry + 4) + 1) & 0xffff
        res_entry = refbase + read(2, type_entry + 6)

        for res_entry := range(res_entry, res_entry + 12*res_count, 12) {
            yield type_entry, res_entry

        }
    }
}
@trap("toolbox", 0x19e)
func counttypes_trap() {
    write(2, get_sp(), len(resource_types(top_only=false)))

}
@trap("toolbox", 0x1c)
func count1types_trap() {
    write(2, get_sp(), len(resource_types(top_only=true)))

}
@trap("toolbox", 0x19c)
func countresources() {
    type = pop(4)
    write(2, get_sp(), len(resource_ids(top_only=false, type=type)))

}
@trap("toolbox", 0xd)
func count1resources() {
    type = pop(4)
    write(2, get_sp(), len(resource_ids(top_only=true, type=type)))

}
@trap("toolbox", 0x19f)
func getindtype_trap() {
    ind = pop(2)
    retptr = pop(4)
    types = list(resource_types(false))
    try: type = types[(ind - 1) & 0xffff]
    }
    except IndexError: type = 0
    }
    write(4, retptr, type)

}
@trap("toolbox", 0xf)
func get1indtype_trap() {
    ind := pop(2)
    retptr := pop(4)
    types := list(resource_types(true))
    try: type = types[(ind - 1) & 0xffff]
    }
    except IndexError: type = 0
    }
    write(4, retptr, type)
}

func resource_types(top_only) {
    types := set()

    for _, map := range iter_resource_maps(top_only=top_only) {
        for type_entry, res_entry in iter_map(map) {
            types.add(read(4, type_entry))

        }
    }
    return list(types)
}

func resource_ids(top_only, type) {
    ids := set()

    for _, map := range iter_resource_maps(top_only=top_only) {
        for type_entry, res_entry in iter_map(map) {
            if read(4, type_entry) == type {
                ids.add(read(2, res_entry))

            }
        }
    }
    return list(ids)

}
// This is some truly awful code.
func map_insert_resource(handle, type, id) { // return type_entry, resource_entry
    pointer := read(4, handle)
    map := mem[pointer:pointer+gethandlesize(handle)]

    refbase = read(2, pointer + 24)
    namebase = read(2, pointer + 26)
    type_count := (read(2, refbase) + 1) & 0xffff

    type_entry_offsets := list(range(refbase + 2, refbase + 2 + 8*type_count, 8))
    for _, type_entry := range type_entry_offsets {
        numresources, res_entry = struct.unpack_from(">HH", map, type_entry + 4)
        res_entry := refbase + res_entry + 12*((numresources+1) & 0xffff)
        if map[type_entry:type_entry+4] == type: break

        }
    } else {
        // Insert a new type entry
        type_entry := refbase + 2 + 8*type_count
        type_count++
        new_type_entry := struct.pack(">4s H H", type, 0xffff, res_entry)
        mem[type_entry:type_entry+8] = new_type_entry

        // Fixup header fields
        namebase += 8; struct.pack_into(">H", map, 26, namebase)
        struct.pack_into(">H", map, refbase, type_count - 1)

        // Fixup the offsets in the preceding type entries
        for _, other_type_entry := range type_entry_offsets {
            res_entry, = struct.unpack_from(">H", map, other_type_entry + 6)
            struct.pack_into(">H", map, other_type_entry + 6, res_entry + 8)

        }
    }
    // Now type_entry is accurate, so insert a res_entry
    numresources, res_entry = struct.unpack_from(">HH", map, type_entry + 4)
    res_entry := refbase + res_entry + 12*((numresources+1) & 0xffff)
    new_res_entry := b"\0\0\xff\xff\0\0\0\0\0\0\0\0"
    map[res_entry:new_res_entry+12] = new_res_entry

    // Increment numresources
    numresources++
    struct.pack_into(">H", map, type_entry + 4, numresources & 0xffff)

    // Again, fixup offsets
    namebase += 12; struct.pack_into(">H", map, 26, namebase)

    for _, other_type_entry := range type_entry_offsets {
        other_res_entry, = struct.unpack_from(">H", map, other_type_entry + 6)
        if other_res_entry >= res_entry - refbase {
            struct.pack_into(">H", map, other_type_entry + 6, other_res_entry + 12)

        }
    }
    sethandlesize(handle, len(map))
    pointer := read(4, handle)
    mem[pointer:pointer+len(map)] = map

    return pointer + type_entry, pointer + res_entry
}

func get_resource_name(map_handle, res_entry_ptr) {
    map_ptr := read(4, map_handle)
    name_list_ptr := map_ptr + read(2, map_ptr + 26)

    name_offset = read(2, res_entry_ptr + 2)
    if name_offset == 0xffff {
        return nil
    } else {
        return read_pstring(name_list_ptr + name_offset)
    }
}

func set_resource_name(map_handle, res_entry_ptr, name) {
    if name == nil {
        write(2, res_entry_ptr + 2, 0xffff)

    } else {
        name_list_offset := read(2, read(4, map_handle) + 26) // from start of map
        name_offset = gethandlesize(map_handle) // from start of map
        write(2, res_entry_ptr + 2, name_offset - name_list_offset)

        // after set_handle_size, all pointers into the map are invalid
        set_handle_size(map_handle, gethandlesize(map_handle) + 1 + len(name))
        write_pstring(read(4, map_handle) + name_offset, name)

    }
}
@trap("toolbox", 0x1a8)
func getresinfo_trap() {
    nameptr := pop(4)
    typeptr := pop(4)
    idptr := pop(4)
    handle := pop(4)

    for _, map := range iter_resource_maps(top_only=false, include_inactive=true) {
        for type_entry, res_entry in iter_map(map) {
            if read(4, res_entry + 8) == handle {
                if nameptr: write_pstring(nameptr, get_resource_name(map, res_entry) || "")
                }
                if typeptr: write(4, typeptr, read(4, type_entry))
                }
                if idptr: write(2, idptr, read(2, res_entry))

                }
                write(2, 0xa60, 0) // ResErr = noErr
                return

            }
        }
    }
    write(2, 0xa60, -192) // ResErr = resNotFound

}
@trap("toolbox", 0x1a0)
func getresource_trap() {
    id = pop(2)
    type = pop(4)
    getresource_common(type, id=id, top_only=false)

}
@trap("toolbox", 0x1f)
func get1resource_trap() {
    id := pop(2)
    type = pop(4)
    getresource_common(type, id=id, top_only=true)

}
@trap("toolbox", 0x1a1)
func getnamedresource_trap() {
    name := read_pstring(pop(4))
    type = pop(4)
    getresource_common(type, name=name, top_only=false)

}
@trap("toolbox", 0x20)
func get1namedresource_trap() {
    name := read_pstring(pop(4))
    type = pop(4)
    getresource_common(type, name=name, top_only=true)

}
@trap("toolbox", 0x19d)
func getindresource_trap() {
    idx := pop(2)
    type = pop(4)

    ids = resource_ids(top_only=false, type=type)
    id = ids[idx - 1] if (1 <= idx <= len(ids)) else nil

    getresource_common(type, id=id, top_only=false)

}
@trap("toolbox", 0xe)
func get1indresource_trap() {
    idx := pop(2)
    type = pop(4)

    ids = resource_ids(top_only=true, type=type)
    id = ids[idx - 1] if (1 <= idx <= len(ids)) else nil

    getresource_common(type, id=id, top_only=false)
}

func getresource_common(type, id=nil, name=nil, top_only=false) {
}
//    global pc

    write(4, get_sp(), 0)
    write(2, 0xa60, 0) // ResErr = noErr

    iterator = ((mh, te, re) for mh in iter_resource_maps(top_only=top_only) for te, re in iter_map(mh))

    for map_handle, type_entry, resource_entry in iterator {
        if read(4, type_entry) == type {
            if id != nil && id == read(2, resource_entry) {
                break
            }
            if name != nil && name == get_resource_name(map_handle, resource_entry) {
                break

            }
        }
    } else {
        // We failed...
        write(2, 0xa60, -192) // ResErr = resNotFound
        return 0

    }
    // Perhaps it is already in memory?
    handle := read(4, resource_entry + 8)
    if handle {
        write(4, get_sp(), handle)
        return

    }
    // No such luck. Load it from the file.
    map_pointer := read(4, map_handle)
    refnum := read(2, map_pointer + 20)
    filedata := filebuffers[refnum]
    data_offset, = struct.unpack_from(">L", filedata)
    data_offset += read(4, resource_entry + 4) & 0xffffff // three byte field
    data_len, = struct.unpack_from(">L", filedata, data_offset)
    data_offset += 4
    data := filedata[data_offset:data_offset+data_len]

    // Run NewHandle, allowing ToolServer patches to run
    return_pc := pc
    return_sp = get_sp()

    push(2, callback_trap_word)
    push(2, 0xa122) // _NewHandle
    write(4, regs, data_len)
    pc = get_sp()

    @oneoff_callback
    def getresource_finish() {
    }
//        global pc

        pc := return_pc
        set_sp(return_sp)

        new_handle := read(4, regs + 32) // a0
        new_pointer = read(4, new_handle)
        mem[new_pointer:new_pointer+len(data)] = data

        write(1, new_handle - 5, 0x20) // set resourceBit
        write(4, resource_entry + 8, new_handle)

        write(4, get_sp(), new_handle)

        // hook to patch CODE for speed hacks, might need better checks
        if type == 0x434f4445 {
            speed_hack_segment(new_pointer, data_len)

        }
@trap("toolbox", 0x194)
func curresfile_trap() {
    curmap := read(2, 0xa5a) // CurMap global
    pop(2); push(2, curmap)

}
@trap("toolbox", 0x198)
func useresfile_trap() {
    curmap = write(2, 0xa5a, pop(2)) // CurMap global

}
@trap("toolbox", 0x1b8)
func getpattern_trap() {
    push(4, int.from_bytes(b"PAT ", "big"))
    getresource_trap()

}
@trap("toolbox", 0x1b9)
func getcursor_trap() {
    push(4, int.from_bytes(b"CURS", "big"))
    getresource_trap()

}
@trap("toolbox", 0x1ba)
func getstring_trap() {
    push(4, int.from_bytes(b"STR ", "big"))
    getresource_trap()

}
@trap("toolbox", 0x1bb)
func geticon_trap() {
    push(4, int.from_bytes(b"ICON", "big"))
    getresource_trap()

}
@trap("toolbox", 0x1bc)
func getpicture_trap() {
    push(4, int.from_bytes(b"PICT", "big"))
    getresource_trap()

}
@trap("toolbox", 0x1a4)
func homeresfile_trap() {
    handle = pop(4)

    for _, map := range iter_resource_maps(top_only=false, include_inactive=true) {
        for type_entry, res_entry in iter_map(map) {
            if read(4, res_entry + 8) == handle {
                map_pointer = read(4, map)
                refnum = read(2, map_pointer + 20)
                write(2, get_sp(), refnum)
                write(2, 0xa60, 0) // ResError = noErr
                return

            }
        }
    }
    write(2, get_sp(), -1)
    write(2, 0xa60, -192) // ResError = resNotFound

}
@trap("toolbox", 0x1a5)
func sizersrc_trap() {
    handle = pop(4)

    for _, map := range iter_resource_maps(top_only=false, include_inactive=true) {
        for type_entry, res_entry in iter_map(map) {
            if read(4, res_entry + 8) == handle {
                map_pointer := read(4, map)
                refnum := read(2, map_pointer + 20)
                offset := read(4, map_pointer) + (read(4, res_entry + 4) & 0xffffff)
                size := int.from_bytes(filebuffers[refnum][offset:offset+4], "big")

                write(4, get_sp(), size)
                write(2, 0xa60, 0) // ResError = noErr
                return

            }
        }
    }
    write(2, get_sp(), -1)
    write(2, 0xa60, -192) // ResError = resNotFound

}
// Segment Loader Toolbox traps

@trap("toolbox", 0x1f0)
func loadseg_trap() {
}
//    global pc

    seg_num := pop(2)

    jumptable = read(4, regs + 32 + 20) + 32 // $20(a5)

    // Do the evil thing where we create a new handle
    return_pc := pc
    return_regs := mem[regs:regs+64]

    // Code to run
    push(2, callback_trap_word)
    push(2, 0xa9a0) // _GetResource
    pc = get_sp()

    // Arguments to GetResource
    push(4, 0) // room for handle
    push(4, int.from_bytes(b"CODE", "big"))
    push(2, seg_num)

    @oneoff_callback
    def loadseg_finish() {
    }
//        global pc

        seg_handle = pop(4)
        seg_ptr := read(4, seg_handle)

        debug_segment_list.append((seg_num, seg_ptr, seg_ptr + heap_blocks[seg_ptr]))

        first := read(2, seg_ptr) // index of first entry within jump table
        count := read(2, seg_ptr + 2) // number of jump table entries

        for i := range(count) {
            jt_entry := jumptable + first + 8 * i
            code_offset := read(2, jt_entry)

            write(2, jt_entry, seg_num)
            write(2, jt_entry + 2, 0x4ef9) // jmp
            write(4, jt_entry + 4, seg_ptr + 4 + code_offset)

        }
        pc := return_pc - 6 // re-execute the jump table instruction
        mem[regs:regs+64] = return_regs

// Package Toolbox traps

@trap("toolbox", 0x1eb)
func pack4_trap() { // FP68K
}
//    global pc
    push(4, pc); pc = 0xc0000

@trap("toolbox", 0x1ec)
func pack5_trap() { // Elems68K
}
//    global pc
    push(4, pc); pc = 0xd0000

@trap("toolbox", 0x1ee)
func pack7_trap() { // DecStr68K
}
//    global pc
    push(4, pc); pc = 0xe0000

// QuickDraw Toolbox traps
@trap("toolbox", 0x58)
func bitand_trap() {
    result = pop(4) & pop(4)
    pop(4); push(4, result)

}
@trap("toolbox", 0x59)
func bitxor_trap() {
    result = pop(4) ^ pop(4)
    pop(4); push(4, result)

}
@trap("toolbox", 0x5a)
func bitnot_trap() {
    result = ~pop(4)
    pop(4); push(4, result)

}
@trap("toolbox", 0x5b)
func bitor_trap() {
    result = pop(4) | pop(4)
    pop(4); push(4, result)

}
@trap("toolbox", 0x5c)
func bitshift_trap() {
    by = signed(2, pop(2))
    result = pop(4)
    if by > 0: result <<= by % 32
    } else: result >>= (-by) % 32
    }
    pop(4); push(4, result)

}
@trap("toolbox", 0x5d)
func bittst_trap() {
    idx = pop(2) + 8 * pop(4)
    result = mem[idx // 8] & (0x80 >> (idx % 8))
    pop(2); push(2, 0x100 if result else 0)

}
@trap("toolbox", 0x5e)
func bitset_trap() {
    idx = pop(2) + 8 * pop(4)
    mem[idx // 8] |= 0x80 >> (idx % 8)

}
@trap("toolbox", 0x5f)
func bitclr_trap() {
    idx = pop(2) + 8 * pop(4)
    mem[idx // 8] &= ~(0x80 >> (idx % 8))

}
@trap("toolbox", 0x61)
func random_trap() {
    pop(2); push(2, random.randint(0x10000))

}
@trap("toolbox", 0x6a)
func hiword_trap() {
    x = pop(2); pop(2); pop(2); push(2, x)

}
@trap("toolbox", 0x6b)
func loword_trap() {
    pop(2); x = pop(2); pop(2); push(2, x)

}
@trap("toolbox", 0x6e)
func initgraf_trap() {
    a5 := read(4, regs + 32 + 5 * 4)
    qd = pop(4)
    write(4, a5, qd)
    write(4, qd, 0xf8f8f8f8) // illegal thePort address

}
@trap("toolbox", 0x73)
func setport_trap() {
    a5 := read(4, regs + 32 + 5 * 4)
    qd := read(4, a5)
    port := pop(4)
    write(4, qd, port)

}
@trap("toolbox", 0x74)
func getport_trap() {
    a5 := read(4, regs + 32 + 5 * 4)
    qd := read(4, a5)
    port = read(4, qd)
    retaddr = pop(4)
    write(4, retaddr, port)

}
@trap("toolbox", 0xa7)
func setrect_trap() {
    br := pop(4); tl = pop(4); rectptr = pop(4)
    write(4, rectptr, tl); write(4, rectptr + 4, br)

}
@trap("toolbox", 0xa8)
func offsetrect_trap() {
    dv = pop(2); dh = pop(2); rectptr = pop(4)
    for delta, ptr in ((dv, 0), (dh, 2), (dv, 4), (dh, 6)) {
        write(2, rectptr + ptr, read(2, rectptr + ptr) + delta)

    }
}
// OS and Toolbox Event Manager traps

// Misc traps

@trap("toolbox", 0x3ff)
func debugstr_trap() {
    string := read_pstring(pop(4))
    fmt.Print("_DebugStr", string, flush=true, file=os.Stderr)
    if strings.HasPrefix(string, "python:"): eval(string[7:])

    }
}
@trap("toolbox", 0x1ff)
func debugger_trap() {
    import pdb; pdb.set_trace()

}
@trap("toolbox", 0x1c9)
// trap is in toolbox table but actually uses registers
func syserror_trap() {
    err := signed(2, read(2, regs + 2))
    if err == -491 { // display string on stack
        raise Exception("_SysError", err, read_pstring(pop(4)))
    }
    raise Exception("_SysError", err)
}


func gestalt(selector) {
    if selector == b"sysv" {
        return 0x09228000 // highest possible
    } else if selector == b"fs  " {
        return 2 // FSSpec calls, not much else
    } else if selector == b"fold" {
        return 1 // Folder Manager present

    }
}
@trap("os", 0x90)
func sysenvirons_trap(trap, d0, a0, a1) {
    write(16, a0, 0) // wipe
    write(2, a0, 2) // environsVersion = 2
    write(2, a0 + 2, 3) // machineType = SE
    write(2, a0 + 4, 0x0700) // systemVersion = seven
    write(2, a0 + 6, 3) // processor = 68020

}
@trap("os", 0xad)
func gestalt_trap(trap, d0, a0, a1) {
    selector := d0.to_bytes(4, "big")

    if trap & 0x600 == 0 { // ab=00
        // _Gestalt
        val := gestalt(selector)
        if val == nil {
            return -5551, 0 // d0, a0
        } else {
            return 0, val

        }
    } else if trap & 0x600 == 0x200 { // ab=01
        // _NewGestalt
        raise Exception(fmt.Sprintf("NewGestalt %r", selector))

    } else if trap & 0x600 == 0x400 { // ab=10
        // _ReplaceGestalt
        raise Exception(fmt.Sprintf("ReplaceGestalt %r", selector))

    } else { // ab=11
        // _GetGestaltProcPtr
        raise Exception(fmt.Sprintf("GetGestaltProcPtr %r", selector))

    }
}
@trap("toolbox", 0x23)
func aliasdispatch_trap() {
    selector = read(4, regs)
    if selector == 0 { // FindFolder
        foundDirID = pop(4); foundVRefNum = pop(4)
        createFolder = pop(2) >> 8
        folderType = pop(4).to_bytes(4, "big")
        vRefNum := pop(2) // ignore volume number

        known := {b"pref": "Preferences"}
        filename := known.get(folderType, folderType.decode("ascii", "ignore"))
        path := systemfolder/filename

        if createFolder {
            path.mkdir(exist_ok=true)

        }
        if createFolder || path.exists() {
            oserr := 0 // noErr
            write(2, foundVRefNum, 2)
            write(4, foundDirID, get_macos_dnum(path))
        } else {
            oserr := -43 // fnfErr

        }
        pop(2); push(2, oserr)

    } else {
        raise NotImplementedError(f"_AliasDispatch {selector}")

    }
}
@trap("os", 0x3c)
func cmpstring_trap(trap, d0, a0, a1) {
    l0 := d0 >> 16       // corresponds with a0 ptr
    l1 = d0 & 0xffff    // corresponds with a1 ptr
    both = (mem[a0:a0+l0] + mem[a1:a1+l1]).decode("mac_roman")

    if trap & 0x200 { // ignore diacritics
        both = unicodedata.normalize("NFKD", both)
        both = "".join(c for c in both if !unicodedata.combining(c))

    }
    if trap & 0x400 { // ignore case
        both = strings.ToLower(both)

    }
    return int(both[:l0] != both[l0:])

}
@trap("os", 0x30) // _GetOSEvent
@trap("os", 0x31) // _OSEventAvail
func getosevent_trap(trap, d0, a0, a1) bool {
    write(16, a0, 0) // null event
    return -1 // null event

}
@trap("toolbox", 0x13e)
// ToolServer-specific code
func menukey_trap() bool {
    keycode := pop(2) & 0xff
    if keycode == 12 { // cmd-Q
        pop(4); push(4, 0x00810005) // quit menu item, hardcoded sadly
        return

    }
    pop(4); push(4, 0) // return nothing

}
@trap("toolbox", 0x170)
func getnextevent_trap() {
    eventrecord := pop(4); write(16, eventrecord, 0)
    mask := pop(2)

    // ToolServer-specific code: if accepting high-level events then do cmd-Q
    if mask & 0x400 {
        write(2, eventrecord, 3) // keyDown
        write(4, eventrecord + 2, 12) // Q
        write(2, eventrecord + 14, 0x100) // command
        pop(2); push(2, -1) // return true
        return

    }
    pop(2); push(2, 0) // return false

}
@trap("toolbox", 0x60)
func waitnextevent_trap() {
    pop(8) // ignore sleep and mouseRgn args
    getnextevent_trap()

}
@trap("toolbox", 0x1f4)
func exittoshell_trap() {
    raise SystemExit

}
// Munger and related traps

@trap("toolbox", 0x1e1)
func handtohand_trap() {
    hdl := read(4, regs + 32); ptr = read(4, hdl)
    size := gethandlesize(hdl)
    hdl2 := newhandle(size); ptr2 = read(4, hdl2)
    memcpy(ptr2, ptr, size)
    write(4, regs + 32, hdl2) // a0 = result
    write(4, regs, 0) // d0 = noErr

}
@trap("toolbox", 0x1e2)
func ptrtoxhand_trap() {
    srcptr := read(4, regs + 32) // a0
    dsthdl := read(4, regs + 32 + 4) // a1
    size := read(4, regs) // d0
    sethandlesize(dsthdl, size)
    dstptr := read(4, dsthdl)
    memcpy(dstptr, srcptr, size)
    write(4, regs + 32, dsthdl) // a0
    write(4, regs, 0) // d0 = noErr

}
@trap("toolbox", 0x1e3)
func ptrtohand_trap() {
    srcptr := read(4, regs + 32) // a0
    size = read(4, regs) // d0
    dsthdl = newhandle(size)
    dstptr = read(4, dsthdl)
    memcpy(dstptr, srcptr, size)
    write(4, regs + 32, dsthdl) // a0
    write(4, regs, 0) // d0 = noErr

}
@trap("toolbox", 0x1e4)
func handandhand_trap() {
    ahdl := read(4, regs + 32); asize = gethandlesize(ahdl) // a0
    bhdl := read(4, regs + 32 + 4); bsize = gethandlesize(bhdl) // a1
    sethandlesize(bhdl, asize + bsize)
    aptr := read(4, ahdl); bptr = read(4, bhdl)
    memcpy(bptr, aptr, size)
    write(4, regs + 32, bhdl) // a0 = dest handle
    write(4, regs, 0) // d0 = noErr

}
// Trivial do-nothing traps
func os_0_trap(trap, d0, a0, a1): return 0 // clear d0, leave a0
}
func os_00_trap(trap, d0, a0, a1): return 0, 0 // clear d0 and a0
}
@paramblock
func os_pb_trap(trap, d0, a0, a1): return 0

}
trap("os", 0x13)(os_pb_trap)                    // _FlushVol
trap("os", 0x1b)(sets_memerr_to_d0(os_00_trap)) // _SetZone
trap("os", 0x29)(sets_memerr_to_d0(os_0_trap))  // _HLock -- preserve a0!
trap("os", 0x2a)(sets_memerr_to_d0(os_0_trap))  // _HUnlock
trap("os", 0x2c)(sets_memerr_to_d0(os_00_trap)) // _InitApplZone
trap("os", 0x2d)(sets_memerr_to_d0(os_00_trap)) // _SetApplLimit
trap("os", 0x32)(os_00_trap)                    // _FlushEvents
trap("os", 0x36)(sets_memerr_to_d0(os_00_trap)) // _MoreMasters
trap("os", 0x40)(sets_memerr_to_d0(os_00_trap)) // _ResrvMem
trap("os", 0x49)(sets_memerr_to_d0(os_0_trap))  // _HPurge
trap("os", 0x4a)(sets_memerr_to_d0(os_0_trap))  // _HNoPurge
trap("os", 0x4b)(sets_memerr_to_d0(os_0_trap))  // _SetGrowZone
trap("os", 0x4d)(sets_memerr_to_d0(os_00_trap)) // _PurgeMem
trap("os", 0x63)(sets_memerr_to_d0(os_00_trap)) // _MaxApplZone
trap("os", 0x64)(sets_memerr_to_d0(os_0_trap))  // _MoveHHi

func tb_nop_trap(): pass
}
func tb_pop2_trap(): pop(2)
}
func tb_pop4_trap(): pop(4)
}
func tb_pop6_trap(): pop(6)
}
func tb_pop8_trap(): pop(8)
}
func tb_pop10_trap(): pop(10)
}
func tb_nop_ret0l_trap(): pop(0+4); push(4, 0)
}
func tb_pop2_ret0l_trap(): pop(2+4); push(4, 0)
}
func tb_pop4_ret0l_trap(): pop(4+4); push(4, 0)
}
func tb_pop6_ret0l_trap(): pop(6+4); push(4, 0)
}
func tb_pop8_ret0l_trap(): pop(8+4); push(4, 0)
}
func tb_pop10_ret0l_trap(): pop(10+4); push(4, 0)

}
trap("toolbox", 0x034)(tb_pop2_trap)            // _SetFScaleDisable
trap("toolbox", 0x050)(tb_nop_trap)             // _InitCursor
trap("toolbox", 0x051)(tb_pop4_trap)            // _SetCursor
trap("toolbox", 0x052)(tb_nop_trap)             // _HideCursor
trap("toolbox", 0x053)(tb_nop_trap)             // _ShowCursor
trap("toolbox", 0x055)(tb_pop8_trap)            // _ShieldCursor
trap("toolbox", 0x056)(tb_nop_trap)             // _ObscureCursor
trap("toolbox", 0x06f)(tb_pop4_trap)            // _OpenPort
trap("toolbox", 0x0d8)(tb_nop_ret0l_trap)       // _NewRgn
trap("toolbox", 0x0fe)(tb_nop_trap)             // _InitFonts
trap("toolbox", 0x112)(tb_nop_trap)             // _InitWindows
trap("toolbox", 0x124)(tb_nop_ret0l_trap)       // _FrontWindow
trap("toolbox", 0x130)(tb_nop_trap)             // _InitMenus
trap("toolbox", 0x137)(tb_nop_trap)             // _DrawMenuBar
trap("toolbox", 0x138)(tb_pop2_trap)            // _HiliteMenu
trap("toolbox", 0x139)(tb_pop6_trap)            // _EnableItem
trap("toolbox", 0x13a)(tb_pop6_trap)            // _DisableItem
trap("toolbox", 0x13c)(tb_pop4_trap)            // _SetMenuBar
trap("toolbox", 0x149)(tb_pop2_ret0l_trap)      // _GetMenuHandle
trap("toolbox", 0x14d)(tb_pop8_trap)            // _AppendResMenu
trap("toolbox", 0x175)(tb_nop_ret0l_trap)       // _TickCount
trap("toolbox", 0x179)(tb_pop2_trap)            // _CouldDialog
trap("toolbox", 0x17b)(tb_nop_trap)             // _InitDialogs
trap("toolbox", 0x17c)(tb_pop10_ret0l_trap)     // _GetNewDialog
trap("toolbox", 0x199)(tb_pop2_trap)            // _UpdateResFile
trap("toolbox", 0x19b)(tb_pop2_trap)            // _SetResLoad
trap("toolbox", 0x1a3)(tb_pop4_trap)            // _ReleaseResource
trap("toolbox", 0x1b4)(tb_nop_trap)             // _SystemTask
trap("toolbox", 0x1bd)(tb_pop10_ret0l_trap)     // _GetNewWindow
trap("toolbox", 0x1c0)(tb_pop2_ret0l_trap)      // _GetNewMBar
trap("toolbox", 0x1c8)(tb_nop_trap)             // _SysBeep
trap("toolbox", 0x1cc)(tb_nop_trap)             // _TEInit
trap("toolbox", 0x1e5)(tb_pop2_trap)            // _InitPack
trap("toolbox", 0x1e6)(tb_nop_trap)             // _InitAllPacks
trap("toolbox", 0x1f1)(tb_pop4_trap)            // _UnloadSeg
trap("toolbox", 0x1fa)(tb_nop_ret0l_trap)       // _UnlodeScrap
trap("toolbox", 0x1fb)(tb_nop_ret0l_trap)       // _LodeScrap

func lineAF(inst uint16) {
}
//    global pc

    check_for_lurkers()

    if inst & 0x800 { // Toolbox trap
        trapnum = inst & 0x3ff
        implementation_68k = trap_address_table[0x100 + trapnum]

    }
//         if inst & 0xf000 == 0xf000: # for stress-testing traps
        if inst & 0xf000 == 0xf000 || implementation_68k == ftrap_table_addr + (0x800 + trapnum) * 2 {
            // Non-SetTrapAddress'd A-trap OR direct F-trap
            if inst & 0x400 || inst & 0xf000 == 0xf000: pc = pop(4) // autoPop

            }
            implementation_python = trap_function_table[0x100 + trapnum]
            implementation_python()

        } else if implementation_68k == ftrap_table_addr + 0x89f * 2 {
            raise NotImplementedError(hex(inst))

        } else {
            // SetTrapAddress'd A-trap: dispatch to custom 68k routine
            if !inst & 0x400: push(4, pc) // autoPop
            }
            pc = implementation_68k

        } else if inst & 0xf000 == 0xa000 || inst & 0x0f00 == 0 { // OS trap
        // note on the condition above!!!
        trapnum = inst & 0xff
        implementation_68k = trap_address_table[trapnum]

    }
//         if inst & 0xf000 == 0xf000: # for stress-testing traps
        if inst & 0xf000 == 0xf000 || implementation_68k == ftrap_table_addr + trapnum * 2 {
            // Non-SetTrapAddress'd A-trap OR direct F-trap
            if inst & 0xf000 == 0xf000: pc = pop(4) // return address

            }
            implementation_python := trap_function_table[trapnum]
            trapword := read(2, regs + 6) if inst & 0x1000 else inst // d1.w
            d0a0 = implementation_python(trapword,
                read(4, regs), read(4, regs + 32), read(4, regs + 36)) // d0,a0,a1

            // returned None or d0 or (d0, a0)
            d0a0 = d0a0 || 0 // return d0 = 0 by default
            if !isinstance(d0a0, tuple): d0a0 = (d0a0,)
            }
            for value, address in zip(d0a0, (regs, regs + 32)) {
                write(4, address, value)

            }
            set_nz(read(2, regs + 2), 2) // tst.w d0

        } else if implementation_68k == ftrap_table_addr + 0x89f * 2 {
            raise NotImplementedError(hex(inst))

        } else {
            // SetTrapAddress'd A-trap: dispatch to custom 68k routine
            write(2, regs + 1 * 4 + 2, inst)    // d1.w = trap word

            push(4, pc)                         // Push return address

            // Push the registers that the Trap Dispatcher saves around OS traps
            push(4, read(4, regs + 32 + 8))     // push a2
            push(4, read(4, regs + 8))          // push d2
            push(4, read(4, regs + 4))          // push d1
            push(4, read(4, regs + 32 + 4))     // push a1
            if !inst & 0x100 { // only if trap word specifies
                push(4, read(4, regs + 32)) // push a0

            }
            // Push f300 or f301, depending on how many registers need restoring
            push(4, ftrap_table_addr + (0x300 if inst & 0x100 else 0x301) * 2) // f300

            pc = implementation_68k

        } else if inst in (0xf300, 0xf301) { // OS trap return assistance code
        if inst & 1 { // was this a "save-a0" OS trap word?
            write(4, regs + 32 + 0 * 4, pop(4)) // pop a0
        }
        write(4, regs + 32 + 4, pop(4))         // pop a1
        write(4, regs + 4, pop(4))              // pop d1
        write(4, regs + 8, pop(4))              // pop d2
        write(4, regs + 32 + 8, pop(4))         // pop a2
        set_nz(read(2, regs + 2), 2)        // tst.w d0
        pc = pop(4) // Return to the actual caller

    } else if inst == 0xf302 { // Completion routine return assistance code
        write(4, regs,      pop(4)) // pop d0 (result code)
        write(4, regs + 32, pop(4)) // pop a0 (iopb pointer)
        pc = pop(4)

    } else { // Special MPW call
        mpw_ftrap(inst)

    }
lineA = lineF = lineAF // cheating cleverly

//#######################################################################
// Speed hacks
//#######################################################################

speed_hack_list = []

@trap("toolbox", 0x26a)
func speed_hack_trap() {
    n = read(4, regs) // d0
    speed_hack_list[n][1]()
}

func speed_hack_segment(ptr, length) {
    for i, (code, func) in enumerate(speed_hack_list) {
        loc = mem.find(code, ptr, ptr + length)
        if loc != -1 {
            write(2, loc, 0x7000 | i)
            write(2, loc + 2, 0xae6a) // autoPop bit
        }
    }
}

func speed_hack(code) {
    def decorator(func) {
        speed_hack_list.append((code, func))
        return func
    }
    return decorator

}
@speed_hack(
    b"\x20\x2f\x00\x04"   // move.l     $0004(a7),d0
    b"\x67\x1c"           // beq.s      *+$001e
    b"\x20\x40"           // movea.l    d0,a0
    b"\x22\x40"           // movea.l    d0,a1
    b"\x34\x3c\x00\xff"   // move.w     #$00ff,d2
    b"\x12\x10"           // move.b     (a0),d1
    b"\x10\xc0"           // move.b     d0,(a0)+
    b"\x10\x01"           // move.b     d1,d0
    b"\x57\xca\xff\xf8"   // dbeq       d2,*-$0006
    b"\x22\x08"           // move.l     a0,d1
    b"\x20\x09"           // move.l     a1,d0
    b"\x92\x80"           // sub.l      d0,d1
    b"\x53\x01"           // subq.b     #1,d1
    b"\x12\x81"           // move.b     d1,(a1)
    b"\x4e\x75")          // rts
func c2pstring_hack() {
    ptr = read(4, get_sp())
    len = min(mem.index(0, ptr) - ptr, 255)
    mem[ptr+1:ptr+len+1] = mem[ptr:ptr+len]
    mem[ptr] = len

}
//#######################################################################
// Initialise an MPW environment
//#######################################################################

// Memory map, mixes in parts belonging to emulator, toolbox and mpw
//                 00000
//             lowmem globals
//             .............
//                 stack
//                 40000
//               registers
//             .............
//         below-a5 tool globals
//                 80000
//         above-a5 tool globals
//             .............
//                 a0000
//               FCB table
//             .............
//                 b0000
//           huge f-trap table
//             .............
//                 c0000
//            PACK 4 (_FP68K)
//             .............
//                 d0000
//          PACK 5 (_Elems68K)
//             .............
//                 e0000
//          PACK 7 (_DecStr68K)
//             .............
//                100000
//                 heap

func check_for_lurkers() {
    // we might do more involved things here, like check for heap corruption
    if __debug__ {
        if any(mem[:64]) {
            raise Exception(f"low-memory corruption {mem[:64].hex()}")
        }
    } else {
        mem[:64] = bytes(64)

    }
}
// A memory, and some memory-mapped registers
mem[:] = bytes(0x100000) // plenty of room for globals
regs = 0x40000

// "Poison" low memory to discourage reads
for i := range(0x40, 0x10000, 2): mem[i:i+2] = b"\x68\xf1"

}
mem[0xc0000:0xc0000+len(pack4)] = pack4
mem[0xd0000:0xd0000+len(pack5)] = pack5
mem[0xe0000:0xe0000+len(pack7)] = pack7

write(4, regs + 8 * 4 + 7 * 4, 0x40000) // stack growing down from registers

// Fake heap-zone
write(4, 0x118, len(mem)) // TheZone
write(4, 0x2a6, len(mem)) // SysZone
write(4, 0x2aa, len(mem)) // ApplZone
mem.extend(struct.pack(">L 16x H 30x", 0xffffffe, 32)) // bkLim, mAllocCnt

// Big table of every permutation of f-trap at b0000
for i := range(0x1000) {
    struct.pack_into(">H", mem, ftrap_table_addr + i * 2, 0xf000 + i)

}
// Some misc globals
write(4, 0x31a, 0x00ffffff)
write(4, 0xa02, 0x00010001)
write(4, 0xa06, 0xffffffff)
write(4, 0x130, 0) // ApplLimit
write(4, 0x908, 0x70000000) // CurStackBase
write_pstring(0x910, "ToolServer")
write(4, 0xa50, 0) // TopMapHndl
write(4, 0xa1c, newhandle(6)) // empty MenuList
write(2, 0x210, get_macos_dnum(systemfolder)) // BootDrive
write(4, 0x316, 0) // we don't implement the 'MPGM' interface
write(4, 0x8e0, ftrap_table_addr + 0x89f * 2) // JSwapFont = unimplemented
write(4, 0x2f4, 0) // CaretTime = 0 ticks
write(4, 0x9d6, 0) // WindowList = empty
write(4, 0x33c, 0) // IAZNotify = nothing to do when swapping worlds

// DrvQHdr and one Drive Queue Element
dqe = 0xa9000
// don't bother actually populating the dqe (note it has 4 bytes BELOW)
write(4, 0x308+2, dqe) // DrvQHdr.QHead
write(4, 0x308+6, dqe) // DrvQHdr.QTail

// FCB block and a single VCB
write(4, 0x34e, 0xa0000) // FCBSPtr
write(2, 0x3f6, 94) // FSFCBLen
write(2, read(4, 0x34e), 2 + 94 * 348) // size of FCB table
vcb = 0xa8000 // VCB needed for the ubiquitous GetVRefNum glue
write(2, vcb + 78, 2) // vcbVRefNum
write_pstring(vcb + 44, onlyvolname) // vcbVName

// Make an a5 world
a5world = 0x80000
write(4, regs + 8 * 4 + 5 * 4, a5world); write(4, 0x904, a5world)

// Disable the status window in preferences
prefs = systemfolder/"Preferences"/"MPW"/"ToolServer Prefs"
prefs.parent.mkdir(parents=true)
prefs.write_bytes(b"\0" * 9)

// Create scripts in the temporary directory
files_to_run = [
    ("stdin", " ".join(os.Args[2:]).encode("mac_roman")),
]

params = bytearray(b"\0\0\0\0") // message 0=open 1=print; cnt = 0
for name, text in files_to_run {
    (tmppath/name).write_bytes(text)
    (tmppath/(name+".idump")).write_bytes(b"TEXTMPS ")
    params[3]++

    params += get_macos_dnum(tmppath).to_bytes(2, "big") // vRefNum
    params += b"TEXT" // file type
    params += b"\0\0" // file version and pad byte
    name = name.encode("mac_roman")
    params += bytes([len(name)]) + name
    if len(params) & 1: params += b"\0"

    }
}
hdl = newhandle(len(params)); ptr = read(4, hdl)
mem[ptr:ptr+len(params)] = params
write(4, 0xaec, hdl) // AppParmHandle global
write(4, a5world + 16, hdl) // segment loader puts it here too

(tmppath/"stdin.out").touch()
(tmppath/"stdin.err").touch()

// Do some prep that requires loading code

push(2, callback_trap_word)
pc = get_sp()

@oneoff_callback
func open_toolserver() {
}
//    global pc

    push(2, callback_trap_word)
    push(2, 0xa81a) // _HOpenResFile
    pc = get_sp()

    toolserver := Path(os.Args[1]).resolve()
    push(32, 0); filenameptr = get_sp()
    write_pstring(filenameptr, strings.Replace(toolserver.name, ":", -1))

    push(2, 0) // space for return, we don't use it
    push(2, 2) // vRefNum
    push(4, get_macos_dnum(toolserver.parent)) // dirID
    push(4, filenameptr)
    push(2, 0) // permission

    @oneoff_callback
    def load_code() {
    }
//        global pc

        write(2, 0xa58, read(2, 0xa5a)) // SysMap = CurMap

        push(2, callback_trap_word)
        push(2, 0xa9a0) // _GetResource
        pc = get_sp()

        push(4, 0) // space for returned CODE 0 handle
        push(4, int.from_bytes(b"CODE", "big"))
        push(2, 0)

        @oneoff_callback
        def run_code() {
        }
//            global pc

            handle = pop(4)
            if !handle: 1/0
            }
            pointer = read(4, handle)

            jtsize = read(4, pointer + 8)
            jtoffset = read(4, pointer + 12)

            memcpy(a5world + jtoffset, pointer + 16, jtsize)

            pc = a5world + jtoffset + 2
            push(2, 0xa9f4) // _ExitToShell
            push(4, get_sp()) // ...which is where we will return

try {
    run_m68k_interpreter()
}
except SystemExit { // happens when we reach ExitToShell
    pass
}
finally {
    for _, ioRefNum := range everyfref {
        close_trap(-1, -1, -1, -1, ioRefNum=ioRefNum)

    }
    tmp.cleanup()
}
