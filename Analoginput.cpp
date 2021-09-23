/*
 * Analoginput.cpp
 *
 *  Created on: 2021. aug. 30.
 *      Author: jobbis
 */

#include "Analoginput.h"
#include <string.h>
// #include "main.h"

// Analog_input::Analog_input(const char *input_name)
Analog_input::Analog_input(const char *input_name)
{
	strcpy(_input_name, input_name);

}

void Analog_input::set_sconfig(ADC_ChannelConfTypeDef s_config, ADC_HandleTypeDef hadcxx)
{
	_s_config.Channel = s_config.Channel;
	_s_config.Offset = s_config.Offset;
	_s_config.OffsetNumber = s_config.OffsetNumber;
	_s_config.Rank = s_config.Rank;
	_s_config.SamplingTime = s_config.SamplingTime;
	_s_config.SingleDiff = s_config.SingleDiff;

	if (HAL_ADC_ConfigChannel(&hadcxx, &_s_config) != HAL_OK) // &hadcxx
	{
		Error_Handler();
	}


}

uint32_t Analog_input::get_ui32_value(ADC_HandleTypeDef hadcxx) // ADC_HandleTypeDef hadcxx
{
	uint32_t ui32_value = 0;

//	if (HAL_ADC_ConfigChannel(&hadcxx, &_s_config) != HAL_OK) // &hadcxx
//	{
//		Error_Handler();
//	}

	HAL_ADC_Start(&hadcxx); // hadcxx
	HAL_ADC_PollForConversion(&hadcxx, 1000);
	ui32_value = HAL_ADC_GetValue(&hadcxx); // hadcxx
	HAL_ADC_Stop(&hadcxx); // hadcxx
	return ui32_value;
}

float Analog_input::get_float_value(ADC_HandleTypeDef hadcxx) //ADC_HandleTypeDef hadcxx
{
	float f_value = 0.0;
	uint32_t ui32_value = 0;

	ui32_value = Analog_input::get_ui32_value(hadcxx);
	f_value = (3.3 * ui32_value) / 4095;
	return f_value;
}

void Analog_input::set_minimum_limit(float min_limit)
{
	_min_limit = min_limit;
}

float Analog_input::get_minimum_limit()
{
	return _min_limit;
}

void Analog_input::set_maximum_limit(float max_limit)
{
	_max_limit = max_limit;
}

float Analog_input::get_maximum_limit()
{
	return _max_limit;
}

void Analog_input::set_queue_id(osMessageQueueId_t queue_id)
{
	_queue_id = queue_id;
}

void Analog_input::send_str_message(const char *str_message)
{
	Message_data_struct data;
	// osStatus_t queue_get_state;

	strcpy(data.message, str_message);
	osMessageQueuePut(_queue_id, &data, 0, 0);
	osDelay(10);
}

void Analog_input::send_float_message(float a_float)
{
	const uint32_t buff_size = 6;
	const uint32_t DECIMAL_DIGITS = 4;
	char temp_buffer[buff_size] =
	{ '0' };
	char tails[] =
	{ ' ' };
	Message_data_struct data;

	snprintf_fp(temp_buffer, buff_size, DECIMAL_DIGITS, tails, a_float);

	strcpy(data.message, temp_buffer);
	osMessageQueuePut(_queue_id, &data, 0, 0);
	osDelay(10);

	// send_str_message(temp_buffer);
}

int Analog_input::snprintf_fp(char destination[], size_t available_chars,
		int decimal_digits, char tail[], float source_number)
{
	// https://stackoverflow.com/questions/905928/using-floats-with-sprintf-in-embedded-c
	int chars_used = 0;    // This will be returned.

	if (available_chars > 0)
	{
		// Handle a negative sign.
		if (source_number < 0)
		{
			// Make it positive
			source_number = 0 - source_number;
			destination[0] = '-';
			++chars_used;
		}

		// Handle rounding
		uint64_t zeros = 1;
		for (int i = decimal_digits; i > 0; --i)
			zeros *= 10;

		uint64_t source_num =
				(uint64_t) ((source_number * (float) zeros) + 0.5f);

		// Determine sliding divider max position.
		uint64_t div_amount = zeros;       // Give it a head start
		while ((div_amount * 10) <= source_num)
			div_amount *= 10;

		// Process the digits
		while (div_amount > 0)
		{
			uint64_t whole_number = source_num / div_amount;
			if (chars_used < (int) available_chars)
			{
				destination[chars_used] = '0' + (char) whole_number;
				++chars_used;

				if ((div_amount == zeros) && (zeros > 1))
				{
					destination[chars_used] = '.';
					++chars_used;
				}
			}
			source_num -= (whole_number * div_amount);
			div_amount /= 10;
		}

		// Store the zero.
		destination[chars_used] = 0;

		// See if a tail was specified.
		size_t tail_len = strlen(tail);

		if ((tail_len > 0) && (tail_len + chars_used < available_chars))
		{
			for (size_t i = 0; i <= tail_len; ++i)
				destination[chars_used + i] = tail[i];
			chars_used += tail_len;
		}
	}

	return chars_used;
}

bool Analog_input::do_test(ADC_HandleTypeDef hadcxx)
{
	const uint32_t MS_DELAY = 10;
	bool result0 = false;
	bool result1 = false;
	bool result2 = false;
	float f_value = 0.0;
	// char str_result[128]; // =
	// { '\0' };
	const uint32_t buff_size = 6;
	const uint32_t DECIMAL_DIGITS = 4;
	char temp_buffer[buff_size] =
	{ '\0' };
	char tails[] =
	{ ' ' };

	int i = 0;

	send_str_message("\n");
	send_input_name();
	send_str_message("\nAlsohatar: ");
	send_float_message(get_minimum_limit());
	send_str_message("\nFelsohatar: ");
	send_float_message(get_maximum_limit());
	send_str_message("\n");

	while (i < 3)
	{
		if (i == 0)
		{
			strcpy(_str_result, "   1. ertek: ");
			f_value = Analog_input::get_float_value(hadcxx);
			snprintf_fp(temp_buffer, buff_size, DECIMAL_DIGITS, tails, f_value);
			strcat(_str_result, temp_buffer);
			if ((f_value > Analog_input::get_minimum_limit())
					&& (f_value < Analog_input::get_maximum_limit()))
			{
				result0 = true;
				strcat(_str_result, " Ok\n");
			}
			else
			{
				result0 = false;
				strcat(_str_result, " Nok\n");
			}
			// usart_send_string(str_result);
			send_str_message(_str_result);
		}
		osDelay(MS_DELAY);

		if (i == 1)
		{
			strcpy(_str_result, "   2. ertek: ");
			f_value = Analog_input::get_float_value(hadcxx);
			// f_value = 1.0;
			snprintf_fp(temp_buffer, buff_size, DECIMAL_DIGITS, tails, f_value);
			strcat(_str_result, temp_buffer);
			if ((f_value > Analog_input::get_minimum_limit())
					&& (f_value < Analog_input::get_maximum_limit()))
			{
				result1 = true;
				strcat(_str_result, " Ok\n");
			}
			else
			{
				result1 = false;
				strcat(_str_result, " Nok\n");
			}
			if (result0 != result1)
			{
				strcat(_str_result,
						"Hibas reszeredmeny: 1.ertek != 2.ertek.\n\n");
				i = -1; // Hogy a ciklus elejére ugorjon.
			}
			// usart_send_string(str_result);
			send_str_message(_str_result);
		}
		osDelay(MS_DELAY);

		if (i == 2)
		{
			strcpy(_str_result, "   2. ertek: ");
			f_value = Analog_input::get_float_value(hadcxx);
			// f_value = 1.0;
			snprintf_fp(temp_buffer, buff_size, DECIMAL_DIGITS, tails, f_value);
			strcat(_str_result, temp_buffer);
			if ((f_value > Analog_input::get_minimum_limit())
					&& (f_value < Analog_input::get_maximum_limit()))
			{
				result2 = true;
				strcat(_str_result, " Ok\n");
			}
			else
			{
				result2 = false;
				strcat(_str_result, " Nok\n");
			}
			if (result1 != result2)
			{
				strcat(_str_result,
						"Hibas reszeredmeny: 2.ertek != 3.ertek.\n\n");
				i = -1; // Hogy a ciklus elejére ugorjon.
			}
			// usart_send_string(str_result);
			send_str_message(_str_result);
		}
		osDelay(MS_DELAY); //
		i++;
	}
	return result0;
}

void Analog_input::send_input_name()
{
	Message_data_struct data;

	strcpy(data.message, _input_name);
	osMessageQueuePut(_queue_id, &data, 0, 0);
	osDelay(10);
}

