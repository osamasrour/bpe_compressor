void bpe_v5(LLST* ll_txt, bucket** tokens){

	assert(ll_txt->length > 1);
	size_t max_vocb_size = (size_t)((double)(ll_txt->length) / 156.25f);
	size_t max_freq_allowed = (size_t)((double)(ll_txt->length) / 500000.0f) >= 2 ? (size_t)((double)(ll_txt->length) / 500000.0f) : 2;
	printf("[INFO] max_vocb_size: %llu\n", max_vocb_size);
	printf("[INFO] max_freq_allowed: %llu\n", max_freq_allowed);

	while(ll_txt->length >= max_vocb_size){
		size_t old_pairs_count = (size_t)ll_txt->length;

		// Genrate the pairs_voc table
		char* biggest_freq_key = NULL;
		uint32_t biggest_freq_hash = 0;
		LLNode* current_node = ll_txt->head;

		while(current_node != NULL && current_node->next != NULL){
			char* pair_l = *(char**)(current_node->data);
			assert(pair_l != NULL);
			char* pair_r = *(char**)(current_node->next->data);
			assert(pair_r != NULL);
			size_t temp_cap = strlen(pair_l) + strlen(pair_r);
			char temp[temp_cap + 1];
			strncpy(temp, pair_l, strlen(pair_l));
			strncpy(temp + strlen(pair_l), pair_r, strlen(pair_r));
			temp[temp_cap] = '\0';
			bucket* kv = shgetp_null(*tokens, temp);
			if(kv == NULL){
				info l = {.freq = 1, .hash = hash_string_fnv1a(temp)};
				shput(*tokens, strdup(temp), l);
			}else{
				kv->value.freq++;
				if (biggest_freq_key == NULL || kv->value.freq > shgetp(*tokens, biggest_freq_key)->value.freq){
					biggest_freq_key = kv->key;
					biggest_freq_hash = kv->value.hash;
				}

			}
			current_node = current_node->next;
		}

		if(shgets(*tokens, biggest_freq_key).value.freq <= max_freq_allowed) {
			printf("\n[EXIT] the tokens length didn't change from the last iter: %llu\n", old_pairs_count);
			printf("[INFO] most freq token value: %llu\n", shgets(*tokens, biggest_freq_key).value.freq);
			break;
		}

		current_node = ll_txt->head;
		LLNode *next_node = NULL;
		while(current_node != NULL && current_node->next != NULL){
			char* pair_l = *(char**)(current_node->data);
			assert(pair_l != NULL);
			assert(current_node->next != NULL);
			char* pair_r = *(char**)(current_node->next->data);
			assert(pair_r != NULL);
			size_t temp_cap = strlen(pair_l) + strlen(pair_r);
			char temp_cmp[temp_cap + 1];
			strncpy(temp_cmp, pair_l, strlen(pair_l));
			strncpy(temp_cmp + strlen(pair_l), pair_r, strlen(pair_r));
			temp_cmp[temp_cap] = '\0';
			uint32_t temp_cmp_hash = hash_string_fnv1a(temp_cmp);

			if(temp_cmp_hash == biggest_freq_hash){
				char* new_pair_ptr = strdup(biggest_freq_key);
				free(*(char**)(current_node->data));
				*(char**)current_node->data = new_pair_ptr;
				assert(current_node->next != NULL);
				next_node = current_node->next;
				if(ll_txt->tail == next_node){
					ll_txt->tail = current_node;
				}
				current_node->next = next_node->next;
				free(*(char**)(next_node->data));
				free(next_node->data);
				free(next_node);
				ll_txt->length--;
			}else{
				current_node = current_node->next;
			}
		}


		if(old_pairs_count == (size_t)ll_txt->length){
			printf("\n[EXIT] the tokens length didn't change from the last iter: %llu\n", old_pairs_count);
			printf("[INFO] most freq token value: %llu\n", shgets(*tokens, biggest_freq_key).value.freq);
			break;
		}
		else{
			printf("\r[INFO] Update pairs number from %zu to %llu, changed by %.2lf%%, most freq value: %llu     ",
				old_pairs_count, ll_txt->length,
				((double)ll_txt->length / (double)old_pairs_count) * 100, shgets(*tokens, biggest_freq_key).value.freq);
			old_pairs_count = (size_t)ll_txt->length;

			for(size_t i = 0; i < shlenu(*tokens); i++){
				assert(shgetp_null(*tokens, (*tokens)[i].key) != NULL);
				free((*tokens)[i].key);
			}
			shfree(*tokens);
			// tokens = NULL;
		}
	}
}