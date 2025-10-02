#include "i2c.h"

#include "print.h"
#include "configs.h"

#define I2C_STATUS_MASK         0xF8
#define I2C_OP_WRITE            0x0
#define I2C_OP_READ             0x1
#define I2C_STATUS_START        0x08
#define I2C_STATUS_ADDR_ACK_W   0x18
#define I2C_STATUS_DATA_ACK_W   0x28
#define I2C_STATUS_ADDR_ACK_R   0x40
#define I2C_STATUS_DATA_ACK_R   0x50
#define I2C_STATUS_DATA_NACK_R  0x58
#define I2C_PRESC_OPT_LOW       0x2 // No. 0 in prescaler val 4
#define I2C_PRESC_OPT_HIGH      0x4 // No. 0 in 64 - 4

#define I2C_SDA_PIN             0x4
#define I2C_SDA_PORT            PORTC
#define I2C_SDA_DDR             DDRC  
#define I2C_SCL_PIN             0x5
#define I2C_SCL_PORT            PORTC
#define I2C_SCL_DDR             DDRC

void I2C_set_internal_pullups(void) {
    // set the SCL and SDA pins to pullup
    I2C_SDA_DDR &= ~(0x1 << I2C_SDA_PIN);
    I2C_SDA_PORT |= (0x1 << I2C_SDA_PIN);

    I2C_SCL_DDR &= ~(0x1 << I2C_SCL_PIN);
    I2C_SCL_PORT |= (0x1 << I2C_SCL_PIN);
}

void I2C_init(const uint32_t scl_speed) {
    // Scl speed
    uint8_t counter_setting = ((CPU_F / scl_speed) - 16) >> 1;
    // No prescaler
    TWSR &= ~((0x1 << TWPS1) | (0x1 << TWPS0));
    // Prescaler if can be used with no accuracy reduction
    // Equivalent of (counter_setting % (0x1 << I2C_PRESC_OPT_LOW) == 0)
    if ((counter_setting & ((0x1 << I2C_PRESC_OPT_LOW) - 1)) == 0) {
        TWSR |= (0x1 << TWPS0);
        counter_setting >>= I2C_PRESC_OPT_LOW;
    }
    if ((counter_setting & ((0x1 << I2C_PRESC_OPT_HIGH) - 1)) == 0) {
        TWSR |= (0x1 << TWPS1);
        counter_setting >>= I2C_PRESC_OPT_HIGH;
    }
    // Set speed
    TWBR = counter_setting;

    // TODO? toggle SCL until SDA high to reset line
}

static inline void I2C_stop(void) {
    TWCR = (0x1 << TWINT) | (0x1 << TWSTO) | (0x1 << TWEN);
    // Make sure stop is cleared before continuing
    uint16_t max_cycles = -1;
    while ((TWCR & (0x1 << TWSTO)) && (--max_cycles > 0));
    if (TWCR & (0x1 << TWSTO)) {
        print("I2C stop error %x %x waited: %u", TWCR, TWSR, max_cycles);
    }
}

static inline Result I2C_start(void) {
    TWCR = (0x1 << TWINT)| (0x1 << TWSTA) | (0x1 << TWEN);
    uint16_t max_cycles = -1;
    while (!(TWCR & (0x1 << TWINT)) && (--max_cycles > 0));
    if ((TWSR & I2C_STATUS_MASK) != I2C_STATUS_START) {
        print("I2C start error %x waited: %u", TWSR, max_cycles);
        I2C_stop();
        return ERROR;
    }
    return SUCCESS;
}

void I2C_read(const uint8_t address, uint8_t * data, const uint8_t data_size) {
    if (data == NULL) {
        print("Error nullptr");
        return;
    }

    // Start
    if (I2C_start() != SUCCESS) {
        return;
    }

    // Send address
    TWDR = (address << 1) | I2C_OP_READ;
    TWCR = (0x1 << TWINT) | (0x1 << TWEA) | (0x1 << TWEN);
    uint16_t max_cycles = -1;
    while (!(TWCR & (1 << TWINT)) && (--max_cycles > 0));
    if ((TWSR & I2C_STATUS_MASK) != I2C_STATUS_ADDR_ACK_R) {
        print("I2C send address error %d waited: %u", TWSR, max_cycles);
        I2C_stop();
        return;
    }

    // Receive data
    for (uint8_t i = 0; i < data_size; ++i) {
        TWCR = (0x1 << TWINT) | (0x1 << TWEA) | (0x1 << TWEN);
        max_cycles = -1;
        while (!(TWCR & (1 << TWINT)) && (--max_cycles > 0));
        if ((TWSR & I2C_STATUS_MASK)!= I2C_STATUS_DATA_ACK_R) {
            print("I2C receive data %d error %x waited: %u", i, TWSR, max_cycles);
            I2C_stop();
            return;
        }
        data[i] = TWDR;
    }

    // Nack to stop read
    TWCR = (0x1 << TWINT) | (0x1 << TWEN);
    max_cycles = -1;
    while (!(TWCR & (1 << TWINT)) && (--max_cycles > 0));
    if ((TWSR & I2C_STATUS_MASK)!= I2C_STATUS_DATA_NACK_R) {
        print("I2C Nack read data error %x waited: %u", TWSR, max_cycles);
        I2C_stop();
        return;
    }

    // Stop condition
    I2C_stop();
}

// Data address shift TWDR
void I2C_write(const uint8_t address, const uint8_t * data, const uint8_t data_size) {
    if (data == NULL) {
        print("Error nullptr");
        return;
    }

    // Start
    if (I2C_start() != SUCCESS) {
        return;
    }

    // Send address
    TWDR = (address << 1) | I2C_OP_WRITE;
    TWCR = (0x1 << TWINT) | (0x1 << TWEN);
    uint16_t max_cycles = -1;
    while (!(TWCR & (1 << TWINT)) && (--max_cycles > 0));
    if ((TWSR & I2C_STATUS_MASK) != I2C_STATUS_ADDR_ACK_W) {
        print("I2C send address error %x waited: %u", TWSR, max_cycles);
        I2C_stop();
        return;
    }

    // Send data
    for (uint8_t i = 0; i < data_size; ++i) {
        TWDR = data[i];
        TWCR = (0x1 << TWINT) | (0x1 << TWEN);
        max_cycles = -1;
        while (!(TWCR & (1 << TWINT)) && (--max_cycles > 0));
        if ((TWSR & I2C_STATUS_MASK)!= I2C_STATUS_DATA_ACK_W) {
            print("I2C send data %d error %x waited: %u", i, TWSR, max_cycles);
            I2C_stop();
            return;
        }
    }

    // Stop condition
    I2C_stop();
}