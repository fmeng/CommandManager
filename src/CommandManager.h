#pragma once
#include <map>
#include <functional>

template<typename CommandId_Type, typename CommandData_Type>
class CommandManager {
public:
    using CommandHandler = std::function<void(CommandId_Type commandId, CommandData_Type &command_data, void *p_attach)>
    ;

protected:
    static std::map<CommandId_Type, CommandHandler> m_commandHandlers;
    static CommandData_Type *const p_commandData;

public:
    static void unregisterCommandHandler(CommandId_Type commandId) {
        m_commandHandlers.erase(commandId);
    }

    static void registerCommandHandler(CommandId_Type commandId, const CommandHandler &handler) {
        m_commandHandlers[commandId] = handler;
    }

    static void registerCommandHandlers(
        const std::map<CommandId_Type, CommandHandler> &idHandlerMap) {
        for (const auto &kv: idHandlerMap) {
            m_commandHandlers[kv.first] = kv.second;
        }
    }

    static bool handleCommand(CommandId_Type commandId,
                              CommandData_Type &commandData,
                              void *p_attach) {
        if (m_commandHandlers.find(commandId) == m_commandHandlers.end()) {
            return false;
        }
        m_commandHandlers[commandId](commandId, commandData, p_attach);
        return true;
    }
};

template<typename CommandId_Type, typename CommandData_Type>
std::map<CommandId_Type,
    typename CommandManager<CommandId_Type, CommandData_Type>::CommandHandler>
CommandManager<CommandId_Type, CommandData_Type>::m_commandHandlers{};

template<typename CommandId_Type, typename CommandData_Type>
CommandData_Type *const
        CommandManager<CommandId_Type, CommandData_Type>::p_commandData =
        CommandData_Type::instance();
