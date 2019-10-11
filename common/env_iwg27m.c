/*
 * Copyright (C) 2018 iWave System Technologies Pvt Ltd.
 * Copyright 2018
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * @file env_iwg27m.c 
 *
 * @brief Dynamic Bootarguments Saving Options for Differnet Boot devices
 *
 * @ingroup Auto Bootarguments Saving
 */

#include <memalign.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <sata.h>
#include <malloc.h>
#include <mmc.h>
#include <search.h>
#include <errno.h>
#include <asm/imx-common/boot_mode.h>

/* references to names in env_common.c */
extern uchar default_environment_ssd[];
extern uchar default_environment_msd[];
extern uchar default_environment_emmc[];
extern uchar default_environment_sata[];
                
char * env_name_spec = "NONE";

#ifdef ENV_IS_EMBEDDED
env_t *env_ptr = &environment
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr;
#endif /* ENV_IS_EMBEDDED */

extern int scsi_curr_dev;

/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

__weak int mmc_get_env_addr(struct mmc *mmc, int copy, u32 *env_addr)
{
        *env_addr = CONFIG_ENV_OFFSET;
        return 0;
}

__weak int mmc_get_env_dev(void)
{
        return CONFIG_SYS_MMC_ENV_DEV;
}

uchar env_get_char_spec(int index)
{
        return *((uchar *)(gd->env_addr + index));
}

__weak int sata_get_env_dev(void)
{
        return CONFIG_SYS_SATA_ENV_DEV;
}

int env_init(void)
{
        /* use default */
        gd->env_valid = 1;

        switch (get_boot_device()) {
                case SATA_BOOT:
                                gd->env_addr = (ulong)&default_environment_sata[0];
                                break;
                case MMC1_BOOT:
                                gd->env_addr = (ulong)&default_environment_emmc[0];
                                break;
                case SD2_BOOT :
                                gd->env_addr = (ulong)&default_environment_ssd[0];
                                break;
                case SD3_BOOT :
                                gd->env_addr = (ulong)&default_environment_msd[0];
                                break;
                default:
                                gd->env_addr = (ulong)&default_environment[0];
                                break;
                }

        return 0;
}

static int init_mmc_for_env(struct mmc *mmc)
{
        if (!mmc) {
                printf("No MMC card found\n");
                return -1;
        }

        if (mmc_init(mmc)) {
                printf("MMC init failed\n");
                return  -1;
        }

#ifdef CONFIG_SYS_MMC_ENV_PART
        if (CONFIG_SYS_MMC_ENV_PART != mmc->block_dev.hwpart ) {
                int mmc_env_devno = mmc_get_env_dev();
                struct mmc *mmc = find_mmc_device(mmc_env_devno);

                if (mmc_switch_part(mmc,
                                    CONFIG_SYS_MMC_ENV_PART)) {
                        puts("MMC partition switch failed\n");
                        return -1;
                }
        }
#endif

        return 0;
}

static void fini_mmc_for_env(struct mmc *mmc)
{
#ifdef CONFIG_SYS_MMC_ENV_PART
	int mmc_env_devno = mmc_get_env_dev();
        struct mmc *m = find_mmc_device(mmc_env_devno);
        if (CONFIG_SYS_MMC_ENV_PART != mmc->block_dev.hwpart)
                mmc_switch_part(m,
                                mmc->block_dev.hwpart);
#endif
}

#ifdef CONFIG_CMD_SAVEENV

static inline int write_env_mmc(struct mmc *mmc, unsigned long size,
                            unsigned long offset, const void *buffer)
{
        uint blk_start, blk_cnt, n;
        struct blk_desc *desc = mmc_get_blk_desc(mmc);

        blk_start       = ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
        blk_cnt         = ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

        n = blk_dwrite(desc, blk_start, blk_cnt, (u_char *)buffer);

        return (n == blk_cnt) ? 0 : -1;
}

static inline int write_env_sata(struct blk_desc *sata, unsigned long size,
                            unsigned long offset, void *buffer)
{
        uint blk_start, blk_cnt, n;

        blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
        blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

        n = blk_dwrite(sata, blk_start, blk_cnt, buffer);

        return (n == blk_cnt) ? 0 : -1;
}

int saveenv_mmc(void)
{

        ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
        int mmc_env_devno = mmc_get_env_dev();
        ssize_t len;
        char    *res;
        struct mmc *mmc = find_mmc_device(mmc_env_devno);
        u32     offset;
        int     ret;

        if (init_mmc_for_env(mmc))
                return 1;

        if (mmc_get_env_addr(mmc, 0, &offset)) {
                ret = 1;
                goto fini;
        }

        res = (char *)&env_new->data;
        len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
        if (len < 0) {
                error("Cannot export environment: errno = %d\n", errno);
                ret = 1;
                goto fini;
        }

        env_new->crc = crc32(0, &env_new->data[0], ENV_SIZE);
        printf("Writing to MMC(%d)... ", mmc_env_devno);
        if (write_env_mmc(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
                puts("failed\n");
                ret = 1;
                goto fini;
        }

        puts("done\n");
        ret = 0;

fini:
        fini_mmc_for_env(mmc);
        return ret;
}
int saveenv_sata(void)
{
        ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
        struct blk_desc *sata = NULL;
        int env_sata, ret;
	
	if (scsi_curr_dev == -1){
        	if (sata_init())
                return 1;
		}

        env_sata = sata_get_env_dev();

        sata = blk_get_devnum_by_type(IF_TYPE_SCSI,env_sata);
        if (sata == NULL) {
                printf("Unknown SATA(%d) device for environment!\n",
                       env_sata);
                return 1;
        }

        ret = env_export(env_new);
        if (ret)
                return 1;

        printf("Writing to SATA(%d)...", env_sata);
        if (write_env_sata(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, (u_char *)env_new)) {
                puts("failed\n");
                return 1;
        }

        puts("done\n");
        return 0;
}

int saveenv (void)
{
	int ret=0;
	char boot_dev;
	boot_dev=get_boot_device();
        switch (boot_dev) {
                case SATA_BOOT:
                                ret = saveenv_sata();
                                break;
                case MMC1_BOOT:
                                ret = saveenv_mmc();
                                break;
                case SD2_BOOT :
                                ret = saveenv_mmc();
                                break;
                case SD3_BOOT :
                                ret = saveenv_mmc();
                                break;

                }
        return ret;
}

#endif /* CONFIG_CMD_SAVEENV */

static inline int read_env_mmc(struct mmc *mmc, unsigned long size,
                           unsigned long offset, const void *buffer)
{
        uint blk_start, blk_cnt, n;
        struct blk_desc *desc = mmc_get_blk_desc(mmc);

        blk_start       = ALIGN(offset, mmc->read_bl_len) / mmc->read_bl_len;
        blk_cnt         = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;

        n = blk_dread(desc, blk_start, blk_cnt, (uchar *)buffer);

        return (n == blk_cnt) ? 0 : -1;
}

static inline int read_env_sata(struct blk_desc *sata, unsigned long size,
                           unsigned long offset, void *buffer)
{
        uint blk_start, blk_cnt, n;

        blk_start = ALIGN(offset, sata->blksz) / sata->blksz;
        blk_cnt   = ALIGN(size, sata->blksz) / sata->blksz;

        n = blk_dread(sata, blk_start, blk_cnt, buffer);

        return (n == blk_cnt) ? 0 : -1;
}

void env_relocate_spec_mmc(void)
{
#if !defined(ENV_IS_EMBEDDED)
        ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
        int mmc_env_devno = mmc_get_env_dev();
        struct mmc *mmc = find_mmc_device(mmc_env_devno);
        u32 offset;
        int ret;

        if (init_mmc_for_env(mmc)) {
                ret = 1;
                goto err;
        }

        if (mmc_get_env_addr(mmc, 0, &offset)) {
                ret = 1;
                goto fini;
        }

        if (read_env_mmc(mmc, CONFIG_ENV_SIZE, offset, buf)) {
                ret = 1;
                goto fini;
        }

        env_import(buf, 1);
        ret = 0;

fini:
        fini_mmc_for_env(mmc);
err:
        if (ret)
                set_default_env(NULL);
#endif
}

void env_relocate_spec_sata(void)
{
        ALLOC_CACHE_ALIGN_BUFFER(char, buf, CONFIG_ENV_SIZE);
        struct blk_desc *sata = NULL;
        int env_sata;

	if (scsi_curr_dev == -1){
         	if (sata_init())
                return ;
                }
	
        env_sata = sata_get_env_dev();

        sata = blk_get_devnum_by_type(IF_TYPE_SCSI,env_sata);
        if (sata == NULL) {
                printf("Unknown SATA(%d) device for environment!\n",
                       env_sata);
                return;
        }

        if (read_env_sata(sata, CONFIG_ENV_SIZE, CONFIG_ENV_OFFSET, buf))
                return set_default_env(NULL);

        env_import(buf, 1);
}

void env_relocate_spec(void)
{	
	char boot_dev;
	boot_dev=get_boot_device();
        switch (boot_dev) {
                case SATA_BOOT:
                                strcpy (env_name_spec, "SATA");
                                env_relocate_spec_sata ();
                                break;
                case MMC1_BOOT:
                                strcpy (env_name_spec, "EMMC");
                                env_relocate_spec_mmc ();
                                break;
                case SD2_BOOT :
                                strcpy (env_name_spec, "SSD");
                                env_relocate_spec_mmc ();
                                break;
                case SD3_BOOT :
                                strcpy (env_name_spec, "MSD");
                                env_relocate_spec_mmc ();
                                break;
        }
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
        puts ("*** Warning - bad CRC or MMC, using default environment\n\n");
        set_default_env(NULL);
}
#endif
