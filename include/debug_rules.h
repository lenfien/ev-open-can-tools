#pragma once

#include "can_frame_types.h"
#include "drivers/can_driver.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#include <SPIFFS.h>
#endif

#define DBG_RULES_MAX 32
#define DBG_LOG_MAX   16
#define DBG_RULES_FILE "/dbg_rules.txt"

struct DbgRule
{
    uint32_t canId = 0;
    int8_t mux = -1;    // -1 = match any mux (0-15)
    int8_t bit = 0;     // 0-63
    uint8_t bitVal = 0; // 0 or 1
    bool enabled = true;
    char name[24] = {};  // 用户自定义名称
};

// Per-(canId, actualMux) record of the last frame we sent
struct DbgLogEntry
{
    uint32_t canId = 0;
    int8_t mux = 0;
    uint8_t data[8] = {};
    bool valid = false;
};

static DbgRule dbgRules[DBG_RULES_MAX];
static uint8_t dbgRuleCount = 0;
static bool dbgActive = false;

static volatile DbgLogEntry dbgLog[DBG_LOG_MAX];
static volatile uint8_t dbgLogCount = 0;

// ── Frame processing ──────────────────────────────────────────────

static void dbgProcessFrame(const CanFrame &frame, CanDriver &driver)
{
    if (!dbgActive || dbgRuleCount == 0)
        return;

    bool anyMatch = false;
    for (uint8_t i = 0; i < dbgRuleCount; i++)
    {
        const DbgRule &r = dbgRules[i];
        if (!r.enabled) continue;
        if (r.canId != frame.id) continue;
        if (r.mux >= 0 && (frame.data[0] & 0x07) != (uint8_t)r.mux) continue;
        anyMatch = true;
        break;
    }
    if (!anyMatch)
        return;

    CanFrame modified = frame;
    for (uint8_t i = 0; i < dbgRuleCount; i++)
    {
        const DbgRule &r = dbgRules[i];
        if (!r.enabled) continue;
        if (r.canId != frame.id) continue;
        if (r.mux >= 0 && (frame.data[0] & 0x07) != (uint8_t)r.mux) continue;

        uint8_t byteIdx = (uint8_t)r.bit / 8;
        uint8_t bitIdx = (uint8_t)r.bit % 8;
        if (byteIdx >= 8) continue;

        if (r.bitVal)
            modified.data[byteIdx] |= (1u << bitIdx);
        else
            modified.data[byteIdx] &= ~(1u << bitIdx);
    }
    driver.send(modified);

    // Record last sent frame per (canId, actualMux)
    int8_t actualMux = (int8_t)(modified.data[0] & 0x07);
    volatile DbgLogEntry *entry = nullptr;
    uint8_t cnt = dbgLogCount;
    for (uint8_t i = 0; i < cnt; i++)
    {
        if (dbgLog[i].canId == modified.id && dbgLog[i].mux == actualMux)
        {
            entry = &dbgLog[i];
            break;
        }
    }
    if (!entry && cnt < DBG_LOG_MAX)
    {
        entry = &dbgLog[cnt];
        entry->canId = modified.id;
        entry->mux = actualMux;
        dbgLogCount = cnt + 1;
    }
    if (entry)
    {
        for (uint8_t b = 0; b < 8; b++)
            entry->data[b] = modified.data[b];
        entry->valid = true;
    }
}

// ── SPIFFS persistence ────────────────────────────────────────────

static void dbgSaveRules()
{
    File f = SPIFFS.open(DBG_RULES_FILE, "w");
    if (!f)
        return;
    for (uint8_t i = 0; i < dbgRuleCount; i++)
    {
        const DbgRule &r = dbgRules[i];
        // 格式：canId mux bit val en name（name 中空格替换为 \x01 以免分割出错）
        String safeName = String(r.name);
        safeName.replace(" ", "\x01");
        if (safeName.length() == 0) safeName = "-";
        f.printf("%lx %d %u %u %d %s\n",
                 (unsigned long)r.canId, (int)r.mux,
                 (unsigned)r.bit, (unsigned)r.bitVal,
                 r.enabled ? 1 : 0,
                 safeName.c_str());
    }
    f.close();
}

static void dbgLoadRules()
{
    dbgRuleCount = 0;
    if (!SPIFFS.exists(DBG_RULES_FILE))
        return;
    File f = SPIFFS.open(DBG_RULES_FILE, "r");
    if (!f)
        return;
    while (f.available() && dbgRuleCount < DBG_RULES_MAX)
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;
        unsigned long id;
        int mux, bit, val, en;
        char nameBuf[32] = {};
        int parsed = sscanf(line.c_str(), "%lx %d %d %d %d %31s", &id, &mux, &bit, &val, &en, nameBuf);
        if (parsed >= 5)
        {
            DbgRule &r = dbgRules[dbgRuleCount++];
            r.canId = (uint32_t)id;
            r.mux = (int8_t)mux;
            r.bit = (int8_t)bit;
            r.bitVal = (uint8_t)(val & 1);
            r.enabled = (en != 0);
            if (parsed >= 6 && strcmp(nameBuf, "-") != 0)
            {
                // 还原空格
                for (char *p = nameBuf; *p; p++) if (*p == '\x01') *p = ' ';
                strncpy(r.name, nameBuf, sizeof(r.name) - 1);
            }
        }
    }
    f.close();
}

// ── JSON helpers ─────────────────────────────────────────────────

static String dbgRulesToJson()
{
    String j = "{\"active\":";
    j += dbgActive ? "true" : "false";
    j += ",\"rules\":[";
    for (uint8_t i = 0; i < dbgRuleCount; i++)
    {
        if (i)
            j += ",";
        const DbgRule &r = dbgRules[i];
        j += "{\"id\":" + String(r.canId);
        j += ",\"mux\":" + String((int)r.mux);
        j += ",\"bit\":" + String((int)r.bit);
        j += ",\"val\":" + String((unsigned)r.bitVal);
        j += ",\"en\":" + String(r.enabled ? 1 : 0);
        // name 字段：转义双引号和反斜杠
        j += ",\"name\":\"";
        for (const char *p = r.name; *p; p++)
        {
            if (*p == '"') j += "\\\"";
            else if (*p == '\\') j += "\\\\";
            else j += *p;
        }
        j += "\"";
        j += "}";
    }
    j += "]}";
    return j;
}

static String dbgLogToJson()
{
    String j = "[";
    bool first = true;
    uint8_t cnt = dbgLogCount;
    for (uint8_t i = 0; i < cnt; i++)
    {
        if (!dbgLog[i].valid) continue;
        if (!first) j += ",";
        first = false;
        j += "{\"id\":" + String(dbgLog[i].canId);
        j += ",\"mux\":" + String((int)dbgLog[i].mux);
        j += ",\"data\":[";
        for (uint8_t b = 0; b < 8; b++)
        {
            if (b) j += ",";
            j += String((unsigned)dbgLog[i].data[b]);
        }
        j += "]}";
    }
    j += "]";
    return j;
}

// Parse JSON array: [{"id":N,"mux":N,"bit":N,"val":N,"en":N}, ...]
static bool dbgParseRulesJson(const String &body)
{
    uint8_t count = 0;
    DbgRule temp[DBG_RULES_MAX];

    int pos = 0;
    int len = body.length();
    while (pos < len && count < DBG_RULES_MAX)
    {
        while (pos < len && body[pos] != '{')
            pos++;
        if (pos >= len)
            break;
        int end = body.indexOf('}', pos);
        if (end < 0)
            break;
        String obj = body.substring(pos + 1, end);
        pos = end + 1;

        auto getField = [&](const char *key) -> long {
            String k = String("\"") + key + "\":";
            int kp = obj.indexOf(k);
            if (kp < 0) return LONG_MIN;
            int vp = kp + k.length();
            while (vp < (int)obj.length() && obj[vp] == ' ') vp++;
            return obj.substring(vp).toInt();
        };

        long id  = getField("id");
        long mux = getField("mux");
        long bit = getField("bit");
        long val = getField("val");
        long en  = getField("en");

        if (id == LONG_MIN || mux == LONG_MIN || bit == LONG_MIN ||
            val == LONG_MIN || en == LONG_MIN)
            continue;
        if (id < 1 || id > 0x7FF) continue;
        if (mux < -1 || mux > 15) continue;
        if (bit < -1 || bit > 63) continue;
        if (val < 0 || val > 1)   continue;

        temp[count].canId   = (uint32_t)id;
        temp[count].mux     = (int8_t)mux;
        temp[count].bit     = (uint8_t)bit;
        temp[count].bitVal  = (uint8_t)val;
        temp[count].enabled = (en != 0);
        // 解析 name 字段
        {
            String k = String("\"") + "name" + "\":";
            int kp = obj.indexOf(k);
            if (kp >= 0)
            {
                int vp = kp + k.length();
                while (vp < (int)obj.length() && obj[vp] == ' ') vp++;
                if (vp < (int)obj.length() && obj[vp] == '"')
                {
                    vp++;
                    String nm;
                    while (vp < (int)obj.length() && obj[vp] != '"')
                    {
                        if (obj[vp] == '\\' && vp + 1 < (int)obj.length()) { vp++; nm += obj[vp]; }
                        else nm += obj[vp];
                        vp++;
                    }
                    nm = nm.substring(0, 23);
                    strncpy(temp[count].name, nm.c_str(), sizeof(temp[count].name) - 1);
                }
            }
        }
        count++;
    }

    dbgRuleCount = count;
    memcpy(dbgRules, temp, sizeof(DbgRule) * count);
    return true;
}
