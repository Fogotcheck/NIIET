/**
  ******************************************************************************
  * @file    plib015_flash.c
  *
  * @brief   Файл содержит реализацию функций для работы с FLASH
  *
  * @author  НИИЭТ, Александр Дыхно <dykhno@niiet.ru>
  *
  ******************************************************************************
  * @attention
  *
  * ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ «КАК ЕСТЬ», БЕЗ КАКИХ-ЛИБО
  * ГАРАНТИЙ, ЯВНО ВЫРАЖЕННЫХ ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ ГАРАНТИИ ТОВАРНОЙ
  * ПРИГОДНОСТИ, СООТВЕТСТВИЯ ПО ЕГО КОНКРЕТНОМУ НАЗНАЧЕНИЮ И ОТСУТСТВИЯ
  * НАРУШЕНИЙ, НО НЕ ОГРАНИЧИВАЯСЬ ИМИ. ДАННОЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ
  * ПРЕДНАЗНАЧЕНО ДЛЯ ОЗНАКОМИТЕЛЬНЫХ ЦЕЛЕЙ И НАПРАВЛЕНО ТОЛЬКО НА
  * ПРЕДОСТАВЛЕНИЕ ДОПОЛНИТЕЛЬНОЙ ИНФОРМАЦИИ О ПРОДУКТЕ, С ЦЕЛЬЮ СОХРАНИТЬ ВРЕМЯ
  * ПОТРЕБИТЕЛЮ. НИ В КАКОМ СЛУЧАЕ АВТОРЫ ИЛИ ПРАВООБЛАДАТЕЛИ НЕ НЕСУТ
  * ОТВЕТСТВЕННОСТИ ПО КАКИМ-ЛИБО ИСКАМ, ЗА ПРЯМОЙ ИЛИ КОСВЕННЫЙ УЩЕРБ, ИЛИ
  * ПО ИНЫМ ТРЕБОВАНИЯМ, ВОЗНИКШИМ ИЗ-ЗА ИСПОЛЬЗОВАНИЯ ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ
  * ИЛИ ИНЫХ ДЕЙСТВИЙ С ПРОГРАММНЫМ ОБЕСПЕЧЕНИЕМ.
  *
  * <h2><center>&copy; 2024 ОАО "НИИЭТ"</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "plib015_flash.h"

/** @addtogroup Peripheral
  * @{
  */

/** @addtogroup FLASH
  * @{
  */

/** @defgroup FLASH_Private Приватные данные
  * @{
  */

/** @defgroup FLASH_Private_Defines Приватные определения
  * @{
  */
#define __NOP() asm volatile("nop")

#define __NOP5() \
	__NOP(); \
	__NOP(); \
	__NOP(); \
	__NOP(); \
	__NOP()

/**
  * @}
  */

/** @defgroup FLASH_Private_Functions Приватные функции
  * @{
  */

/**
  * @brief   Чтение информации из выбранной области флеш, начиная с указанного адреса
  * @param   AddrVal  Стартовый адрес
  * @param   DataArr  Указатель на массив, в который будут переданы 2 32-битных слова данных
  * @param   Region  Выбор области
  * @retval  void
  */
void FLASH_ReadData(uint32_t AddrVal, uint32_t *DataArr,
		    FLASH_Region_TypeDef Region)
{
	assert_param(IS_FLASH_REGION(Region));
	if (Region == FLASH_Region_Main)
		assert_param(IS_FLASH_MAIN_ADDR(AddrVal));
	else /* (Region == FLASH_Region_NVR) */
		assert_param(IS_FLASH_NVR_ADDR(AddrVal));

	FLASH_SetAddr(AddrVal);
	FLASH_SetCmd(FLASH_Cmd_Read, Region);
	__NOP5();
	while (FLASH_BusyStatus()) {
	};
	for (uint32_t i = 0; i < MEM_FLASH_BUS_WIDTH_WORDS; i++) {
		DataArr[i] = FLASH_GetData(i);
	}
}

/**
  * @brief   Запись информации в выбранную область флеш, начиная с указанного адреса
  * @param   AddrVal  Стартовый адрес
  * @param   DataArr  Указатель на массив, из которого будут взяты 2 32-битных слова данных
  * @param   Region  Выбор области
  * @retval  void
  */
void FLASH_WriteData(uint32_t AddrVal, uint32_t *DataArr,
		     FLASH_Region_TypeDef Region)
{
	assert_param(IS_FLASH_REGION(Region));
	if (Region == FLASH_Region_Main)
		assert_param(IS_FLASH_MAIN_ADDR(AddrVal));
	else /* (Region == FLASH_Region_NVR) */
		assert_param(IS_FLASH_NVR_ADDR(AddrVal));

	FLASH_SetAddr(AddrVal);
	for (uint32_t i = 0; i < MEM_FLASH_BUS_WIDTH_WORDS; i++) {
		FLASH_SetData(i, DataArr[i]);
	}
	FLASH_SetCmd(FLASH_Cmd_Write, Region);
	__NOP5();
	while (FLASH_BusyStatus()) {
	};
}

/**
  * @brief   Стирание выбранной страницы флеш
  * @param   AddrVal  Начальный адрес страницы
  * @param   Region  Выбор области
  * @retval  void
  */
void FLASH_ErasePage(uint32_t AddrVal, FLASH_Region_TypeDef Region)
{
	assert_param(IS_FLASH_REGION(Region));
	if (Region == FLASH_Region_Main)
		assert_param(IS_FLASH_MAIN_ADDR(AddrVal));
	else /* (Region == FLASH_Region_NVR) */
		assert_param(IS_FLASH_NVR_ADDR(AddrVal));

	FLASH_SetAddr(AddrVal);
	FLASH_SetCmd(FLASH_Cmd_ErasePage, Region);
	__NOP5();
	while (FLASH_BusyStatus()) {
	};
}

/**
  * @brief   Стирание области флеш полностью
  * @param   Region  Выбор области
  * @retval  void
  */
void FLASH_EraseFull(FLASH_Region_TypeDef Region)
{
	assert_param(IS_FLASH_REGION(Region));

	FLASH_SetCmd(FLASH_Cmd_EraseFull, Region);
	__NOP5();
	while (FLASH_BusyStatus()) {
	};
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2024 NIIET *****END OF FILE****/
