#include "RenderGroup.h"
#include "Memory.h"

RenderGroup RenderGroup::Make(MemoryArena* arena, uptr renderBufferSize, u32 commandQueueCapacity) {
    RenderGroup group = {};
    group.commandQueueCapacity = commandQueueCapacity;
    group.commandQueue = (CommandQueueEntry*)PushSize(arena, sizeof(CommandQueueEntry) * commandQueueCapacity);

    group.renderBufferSize = renderBufferSize;
    group.renderBufferFree = renderBufferSize;
    group.renderBuffer = (byte*)PushSize(arena, renderBufferSize);
    group.renderBufferAt = group.renderBuffer;

    return group;
}

void ValidateCommand(RenderGroup* group, RenderCommand type) {
    if ((type == RenderCommand::LinePushVertex) || (type == RenderCommand::LineEnd)) {
        assert(group->pendingLineBatchCommandHeader);
    } else {
        assert(!group->pendingLineBatchCommandHeader);
    }

    if ((type == RenderCommand::PushChunk) || (type == RenderCommand::EndChunkBatch)) {
        assert(group->pendingChunkBatchCommandHeader);
    } else {
        assert(!group->pendingChunkBatchCommandHeader);
    }
}

void* PushRenderData(RenderGroup* group, u32 size, u32 aligment, void* data) {
    uptr padding = 0;
    u32 useAligment = 0;
    byte* currentAt = group->renderBufferAt;

    if (aligment == 0)  {
        useAligment = 1;
    } else {
        useAligment = aligment;
    }

    if ((uptr)currentAt % useAligment != 0) {
        padding = CalculatePadding((uptr)currentAt, useAligment);
    }


    group->renderBufferAt += size + padding;
    group->renderBufferFree -= size + padding;
    byte* nextAt = currentAt + padding;

    assert((uptr)nextAt % (uptr)useAligment == 0);
    assert(size + padding < group->renderBufferFree);

    memcpy(nextAt, data, size);

    return (void*)nextAt;
}

CommandQueueEntry* PushCommandQueueEntry(RenderGroup* renderGroup, CommandQueueEntry cmd)
{
    assert(renderGroup->commandQueueAt < renderGroup->commandQueueCapacity);
    CommandQueueEntry* renderBucketDest = renderGroup->commandQueue + renderGroup->commandQueueAt;
    memcpy(renderBucketDest, &cmd, sizeof(CommandQueueEntry));
    renderGroup->commandQueueAt++;
    return renderBucketDest;
}

void Push(RenderGroup* group, RenderCommandDrawMesh* command) {
    ValidateCommand(group, RenderCommand::DrawMesh);

    auto renderDataPtr = PushRenderData(group, sizeof(RenderCommandDrawMesh), alignof(RenderCommandDrawMesh), command);

    uptr offset = (uptr)renderDataPtr - (uptr)group->renderBuffer;

    CommandQueueEntry entry = {};
    entry.type = RenderCommand::DrawMesh;
    entry.rbOffset = offset;

    PushCommandQueueEntry(group, entry);
}

void Push(RenderGroup* group, RenderCommandSetDirLight* command) {
    ValidateCommand(group, RenderCommand::SetDirLight);
    group->dirLight = command->light;
}

void Push(RenderGroup* group, RenderCommandLineBegin* command) {
    ValidateCommand(group, RenderCommand::LineBegin);

    auto renderDataPtr = PushRenderData(group,  sizeof(RenderCommandLineBegin), alignof(RenderCommandLineBegin), command);

    CommandQueueEntry entry = {};
    entry.type = RenderCommand::LineBegin;
    entry.rbOffset = (uptr)renderDataPtr - (uptr)group->renderBuffer;
    entry.instanceCount = 0;
    auto header = PushCommandQueueEntry(group, entry);
    group->pendingLineBatchCommandHeader = header;
}

void Push(RenderGroup* group, RenderCommandPushLineVertex* command) {
    ValidateCommand(group, RenderCommand::LinePushVertex);

    PushRenderData(group, sizeof(RenderCommandPushLineVertex), alignof(RenderCommandPushLineVertex), command);
    group->pendingLineBatchCommandHeader->instanceCount++;
}

void Push(RenderGroup* group, RenderCommandLineEnd* command) {
    ValidateCommand(group, RenderCommand::LineEnd);
    group->pendingLineBatchCommandHeader = nullptr;
}

void Push(RenderGroup* group, RenderCommandBeginChunkBatch* command) {
    ValidateCommand(group, RenderCommand::BeginChunkBatch);

    CommandQueueEntry entry = {};
    entry.type = RenderCommand::BeginChunkBatch;
    entry.rbOffset = 0;
    entry.instanceCount = 0;
    auto header = PushCommandQueueEntry(group, entry);
    group->pendingChunkBatchCommandHeader = header;
}

void Push(RenderGroup* group, RenderCommandPushChunk* command) {
    ValidateCommand(group, RenderCommand::PushChunk);

    auto ptr = PushRenderData(group, sizeof(RenderCommandPushChunk), alignof(RenderCommandPushChunk), command);
    if (!group->pendingChunkBatchCommandHeader->instanceCount) {
        group->pendingChunkBatchCommandHeader->rbOffset = (uptr)ptr - (uptr)group->renderBuffer;
    }
    group->pendingChunkBatchCommandHeader->instanceCount++;
}

void Push(RenderGroup* group, RenderCommandEndChunkBatch* command) {
    ValidateCommand(group, RenderCommand::EndChunkBatch);
    group->pendingChunkBatchCommandHeader = nullptr;
}

void Reset(RenderGroup* group) {
    group->commandQueueAt = 0;
    group->renderBufferAt = group->renderBuffer;
    group->renderBufferFree = group->renderBufferSize;
}

void DrawAlignedBoxOutline(RenderGroup* renderGroup, v3 min, v3 max, v3 color, f32 lineWidth) {
    RenderCommandLineBegin beginCommand = {};
    beginCommand.color = color;
    beginCommand.width = lineWidth;
    beginCommand.type = RenderCommandLineBegin::Segments;

    Push(renderGroup, &beginCommand);

    RenderCommandPushLineVertex v0Command = {};
    v0Command.vertex = min;
    Push(renderGroup, &v0Command);

    RenderCommandPushLineVertex v1Command = {};
    v1Command.vertex = V3(max.x, min.y, min.z);
    Push(renderGroup, &v1Command);
    Push(renderGroup, &v1Command);

    RenderCommandPushLineVertex v2Command = {};
    v2Command.vertex = V3(max.x, min.y, max.z);
    Push(renderGroup, &v2Command);
    Push(renderGroup, &v2Command);

    RenderCommandPushLineVertex v3Command = {};
    v3Command.vertex = V3(min.x, min.y, max.z);
    Push(renderGroup, &v3Command);
    Push(renderGroup, &v3Command);

    Push(renderGroup, &v0Command);

    RenderCommandPushLineVertex v4Command = {};
    v4Command.vertex = V3(min.x, max.y, min.z);
    Push(renderGroup, &v4Command);

    RenderCommandPushLineVertex v5Command = {};
    v5Command.vertex = V3(max.x, max.y, min.z);
    Push(renderGroup, &v5Command);
    Push(renderGroup, &v5Command);

    RenderCommandPushLineVertex v6Command = {};
    v6Command.vertex = V3(max.x, max.y, max.z);
    Push(renderGroup, &v6Command);
    Push(renderGroup, &v6Command);

    RenderCommandPushLineVertex v7Command = {};
    v7Command.vertex = V3(min.x, max.y, max.z);
    Push(renderGroup, &v7Command);
    Push(renderGroup, &v7Command);

    Push(renderGroup, &v4Command);

    Push(renderGroup, &v0Command);
    Push(renderGroup, &v4Command);

    Push(renderGroup, &v1Command);
    Push(renderGroup, &v5Command);

    Push(renderGroup, &v2Command);
    Push(renderGroup, &v6Command);

    Push(renderGroup, &v3Command);
    Push(renderGroup, &v7Command);

    RenderCommandLineEnd end = {};
    Push(renderGroup, &end);
}

void DrawStraightLine(RenderGroup* renderGroup, v3 begin, v3 end, v3 color, f32 lineWidth) {
    RenderCommandLineBegin beginCommand = {};
    beginCommand.color = color;
    beginCommand.width = lineWidth;
    beginCommand.type = RenderCommandLineBegin::Segments;
    Push(renderGroup, &beginCommand);

    RenderCommandPushLineVertex v0Command = {};
    v0Command.vertex = begin;
    Push(renderGroup, &v0Command);

    RenderCommandPushLineVertex v1Command = {};
    v1Command.vertex = end;
    Push(renderGroup, &v1Command);

    RenderCommandLineEnd endCommand = {};
    Push(renderGroup, &endCommand);
}
