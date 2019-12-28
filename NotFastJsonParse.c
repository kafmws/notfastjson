#define _CRT_SECURE_NO_WARNINGS
#include "JsonUtil.h"
#include "access.h"
#include "parse.h"
#include <string.h>

jboolean parseNode(JNIEnv *env, jclass *jsonBeanClass, nfjson_string *key, nfjson_value *val, jobject*jsonBeanObject) {
	jfieldID fieldID;
	nfjson_type type = nfjson_get_type(val);
	if (type == JSON_NULL || type == JSON_UNRESOLVED) return JNI_FALSE;
	if (type == JSON_TRUE) {
		fieldID = (*env)->GetFieldID(env, *jsonBeanClass, key->s, "Z");
		(*env)->SetBooleanField(env, *jsonBeanObject, fieldID, JNI_TRUE);
	} else if (type == JSON_FALSE) {
		fieldID = (*env)->GetFieldID(env, *jsonBeanClass, key->s, "Z");
		(*env)->SetBooleanField(env, *jsonBeanObject, fieldID, JNI_FALSE);
	} else if (type == JSON_NUMBER) {
		//处理各种数据类型
		fieldID = (*env)->GetFieldID(env, *jsonBeanClass, key->s, "I");
		jint num = (jint)nfjson_get_number(val);
		(*env)->SetIntField(env, *jsonBeanObject, fieldID, num);
	} else if (type == JSON_STRING) {
		fieldID = (*env)->GetFieldID(env, *jsonBeanClass, key->s, "Ljava/lang/String;");
		const char *str = nfjson_get_string(val);
		jstring jstr = (*env)->NewStringUTF(env, str);
		(*env)->SetObjectField(env, *jsonBeanObject, fieldID, jstr);
	} else if (type == JSON_OBJECT) {
		int count = nfjson_get_object_size(val);
		nfjson_string **keys = (nfjson_string **)malloc(count * sizeof(nfjson_string *));
		nfjson_get_object_key(val, keys);
		for (int i = 0; i < count; i++) {
			nfjson_value *value = nfjson_get_object_value(val, keys[i]);
			//value也是json字符串的情况
			if (nfjson_get_type(value) == JSON_OBJECT) {
				jfieldID classNameID = (*env)->GetFieldID(env, *jsonBeanClass, "className", "Ljava/lang/String;");
				jstring jFullClassName = (jstring)((*env)->GetObjectField(env, *jsonBeanObject, classNameID));
				const char *fullClassName = (*env)->GetStringUTFChars(env, jFullClassName, NULL);
				int fullClassNameLen = (int)((*env)->GetStringLength(env, jFullClassName));
				//全限定名长度超过50个字符，停止解析（部分解析成功）
				if (keys[i]->len + fullClassNameLen > 100) return JNI_FALSE;
				char *fullInnerClassName = (char *)malloc(101 * sizeof(char));
				strcpy(fullInnerClassName, fullClassName);
				strcat(fullInnerClassName, "$");
				//防止修改key的值所以声明一份拷贝
				char *innerClassName = (char *)malloc((keys[i]->len + 1 + 4) * sizeof(char));
				if (keys[i]->len > 0 && (keys[i]->s)[0] >= 'a' && (keys[i]->s)[0] <= 'z') {
					(keys[i]->s)[0] -= 32;
					strcpy(innerClassName, keys[i]->s);
					(keys[i]->s)[0] += 32;
				} else if (keys[i]->len > 0) {
					strcpy(innerClassName, keys[i]->s);
				} else {
					//变量名为空，停止解析，以防引起程序崩溃
					return JNI_FALSE;
				}
				strcat(innerClassName, "Bean");
				strcat(fullInnerClassName, innerClassName);
				jclass innerClass = (*env)->FindClass(env, fullInnerClassName);
				jmethodID constructorID = (*env)->GetMethodID(env, innerClass, "<init>", "()V");
				jobject innerInstance = (*env)->NewObject(env, innerClass, constructorID);
				//为Bean对象Object类型数据赋值
				char *signature = (char *)malloc(103 * sizeof(char));
				strcpy(signature, "L");
				strcat(signature, fullInnerClassName);
				strcat(signature, ";");
				jfieldID innerInstanceID = (*env)->GetFieldID(env, *jsonBeanClass, keys[i]->s, signature);
				(*env)->SetObjectField(env, *jsonBeanObject, innerInstanceID, innerInstance);
				//设置新的BeanClass及Bean实例
				parseNode(env, &innerClass, keys[i], value, &innerInstance);
				//释放申请的内存空间
				free(signature);
				free(innerClassName);
				free(fullInnerClassName);
			} else {
				parseNode(env, jsonBeanClass, keys[i], value, jsonBeanObject);
			}
		}
		free(keys);
	} else if (type == JSON_ARRAY) {
		int count = nfjson_get_array_size(val);
		for (int i = 0; i < count; i++) {
			nfjson_value *value = nfjson_get_array_element(val, i);

			parseNode(env, jsonBeanClass, NULL, value, jsonBeanObject);
		}
	}
	return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL Java_utils_JsonUtil_notFastParseJson
(JNIEnv *env, jclass jcls, jstring j_json, jobject jsonBeanObject) {
	//不会对字符数组进行修改，传入NULL
	const char *c_json = (*env)->GetStringUTFChars(env, j_json, NULL);
	jclass jsonBeanClass = (*env)->GetObjectClass(env, jsonBeanObject);
	nfjson_value val;
	nfjson_parse(&val, c_json);
	return parseNode(env, &jsonBeanClass, NULL, &val, &jsonBeanObject);
}